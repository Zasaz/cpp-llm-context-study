"""Compare the cheap include-closure baseline against the other strategies.

S4 selects the active file, the project headers it includes by name, and the
implementation file sharing each header's stem. It needs no graph, no symbol
table and no type resolution. If it matches S3 on correctness at a comparable
prompt size, the symbol graph is not earning its place in the design.
"""

import json
import os
import statistics
import sys

from harness import stats
from harness.report import load, thought_tokens

ORDER = ("S1", "S2", "S3", "S4", "S5")


def cells(campaigns):
    grid = {}
    for path in campaigns:
        for record in load(path):
            key = (record["project"], record["strategy"])
            call = record.get("model_call") or {}
            entry = grid.setdefault(key, {
                "loc": record["project_metrics"]["loc"],
                "ok": 0, "n": 0, "prompt": [], "cost": [], "selected": None,
            })
            entry["n"] += 1
            if (record.get("grade") or {}).get("fully_successful"):
                entry["ok"] += 1
            entry["prompt"].append(call.get("prompt_tokens"))
            entry["cost"].append(record.get("cost_usd"))
            meta = record.get("context_meta") or {}
            entry["selected"] = meta.get("files_selected")
    return grid


def report(grid):
    projects = sorted({p for p, _s in grid}, key=lambda p: grid[(p, "S1")]["loc"])
    strategies = [s for s in ORDER if any((p, s) in grid for p in projects)]

    out = ["cheap include-closure baseline against the graph\n\n"]
    out.append("  %-10s %-5s %5s %7s %9s %11s\n"
               % ("project", "str", "files", "prompt", "success", "cost"))
    totals = {s: [0, 0] for s in strategies}
    for project in projects:
        for strategy in strategies:
            cell = grid.get((project, strategy))
            if not cell:
                continue
            totals[strategy][0] += cell["ok"]
            totals[strategy][1] += cell["n"]
            prompt = [v for v in cell["prompt"] if v is not None]
            cost = [v for v in cell["cost"] if v is not None]
            out.append("  %-10s %-5s %5s %7d %9s %11.5f\n"
                       % (project, strategy, cell["selected"],
                          statistics.mean(prompt) if prompt else 0,
                          "%d/%d" % (cell["ok"], cell["n"]),
                          statistics.mean(cost) if cost else 0.0))
        out.append("\n")
    out.append("  pooled success\n")
    for strategy in strategies:
        ok, n = totals[strategy]
        out.append("    %-4s %d/%d  %5.1f%%\n" % (strategy, ok, n,
                                                  100.0 * ok / n if n else 0.0))

    for challenger in ("S4", "S5"):
        if "S3" not in strategies or challenger not in strategies:
            continue
        out.append("\n  %s against S3, per project\n" % challenger)
        for project in projects:
            a = grid.get((project, "S3"))
            b = grid.get((project, challenger))
            if not a or not b:
                continue
            pa = statistics.mean([v for v in a["prompt"] if v is not None])
            pb = statistics.mean([v for v in b["prompt"] if v is not None])
            ca = statistics.mean([v for v in a["cost"] if v is not None])
            cb = statistics.mean([v for v in b["cost"] if v is not None])
            out.append("    %-10s prompt %5.1f%% of S3, cost %+6.1f%%, "
                       "success %d/%d against %d/%d\n"
                       % (project, 100.0 * pb / pa if pa else 0.0,
                          100.0 * (ca - cb) / ca if ca else 0.0,
                          b["ok"], b["n"], a["ok"], a["n"]))
        blocks = []
        for project in projects:
            a, b = grid.get((project, "S3")), grid.get((project, challenger))
            if a and b:
                blocks.append((a["ok"], b["ok"]))
        if len(blocks) >= 2:
            x = [a for a, _b in blocks]
            y = [b for _a, b in blocks]
            _w, p = stats.wilcoxon_signed_rank(x, y)
            out.append("    accuracy, S3 against %s over %d blocks: "
                       "exact Wilcoxon p %s, rank-biserial %s\n"
                       % (challenger, len(blocks),
                          "---" if p is None else "%.4f" % p,
                          "---" if stats.rank_biserial(x, y) is None
                          else "%+.2f" % stats.rank_biserial(x, y)))
    return "".join(out)


def table(grid):
    projects = sorted({p for p, _s in grid}, key=lambda p: grid[(p, "S1")]["loc"])
    order = [s for s in ORDER if any((p, s) in grid for p in projects)]
    rows = []
    for project in projects:
        first = True
        for strategy in order:
            cell = grid.get((project, strategy))
            if not cell:
                continue
            prompt = [v for v in cell["prompt"] if v is not None]
            rows.append("%s & %s & %s & %d & %d/%d"
                        % (project if first else "", strategy, cell["selected"],
                           statistics.mean(prompt) if prompt else 0,
                           cell["ok"], cell["n"]))
            first = False
        rows.append(None)
    rows.pop()
    totals = []
    for strategy in order:
        ok = sum(grid[(p, strategy)]["ok"] for p in projects if (p, strategy) in grid)
        n = sum(grid[(p, strategy)]["n"] for p in projects if (p, strategy) in grid)
        totals.append("%s %d/%d" % (strategy, ok, n))
    out = ["\\begin{table}[htbp]\n\\centering\n\\small\n",
           "\\begin{tabular}{llrrc}\n\\toprule\n",
           "Project & Strategy & Files & Prompt & Fully successful \\\\\n",
           "\\midrule\n"]
    for row in rows:
        out.append("\\midrule\n" if row is None else row + " \\\\\n")
    out.append("\\bottomrule\n\\end{tabular}\n")
    out.append("\\caption{\\CaptionBaseline}\n\\label{tab:baseline}\n")
    out.append("\\end{table}\n")
    return "".join(out), totals


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    skip = set()
    for flag in ("--figures", "--json"):
        if flag in argv:
            skip.add(argv.index(flag) + 1)
    campaigns = [a for i, a in enumerate(argv)
                 if not a.startswith("--") and i not in skip]
    if not campaigns:
        print("usage: python -m harness.baseline CAMPAIGN_DIR [CAMPAIGN_DIR ...]")
        return 2
    grid = cells(campaigns)
    print(report(grid))
    if "--figures" in argv:
        body, totals = table(grid)
        path = os.path.join(argv[argv.index("--figures") + 1],
                            "ch5-tab-baseline.tex")
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(body)
        print("wrote %s  pooled %s" % (path, ", ".join(totals)))
    if "--json" in argv:
        path = argv[argv.index("--json") + 1]
        with open(path, "w", encoding="utf-8") as fh:
            json.dump({"%s/%s" % k: v for k, v in grid.items()}, fh, indent=2)
        print("wrote %s" % path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
