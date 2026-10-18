import json
import os
import statistics
import sys

from harness import stats

STRATEGIES = ("S1", "S2", "S3")
PAIRS = (("S1", "S3"), ("S1", "S2"), ("S2", "S3"))


def cells(records):
    loc = {}
    grid = {}
    for record in records:
        project = record["project"]
        loc[project] = record["project_metrics"]["loc"]
        key = (project, record["strategy"])
        grid.setdefault(key, []).append(record)
    projects = sorted(loc, key=lambda p: loc[p])
    return projects, loc, grid


def responses(projects, grid):
    def per_cell(project, strategy, fn):
        runs = grid.get((project, strategy))
        return None if not runs else fn(runs)

    def accuracy(runs):
        ok = sum(1 for r in runs
                 if (r.get("grade") or {}).get("fully_successful"))
        return ok / float(len(runs))

    def cost(runs):
        values = [r.get("cost_usd") for r in runs
                  if isinstance(r.get("cost_usd"), (int, float))]
        return statistics.mean(values) if values else None

    def prompt(runs):
        values = [(r.get("model_call") or {}).get("prompt_tokens")
                  for r in runs]
        values = [v for v in values if isinstance(v, (int, float))]
        return statistics.mean(values) if values else None

    out = {}
    for name, fn, better in (("accuracy", accuracy, "higher"),
                             ("cost_usd", cost, "lower"),
                             ("prompt_tokens", prompt, "lower")):
        table = {}
        for project in projects:
            row = [per_cell(project, s, fn) for s in STRATEGIES]
            if all(v is not None for v in row):
                table[project] = row
        out[name] = {"table": table, "better": better}
    return out


def analyse(projects, grid):
    result = {}
    for measure, payload in responses(projects, grid).items():
        table = payload["table"]
        blocks_by_project = [(p, table[p]) for p in projects if p in table]
        blocks = [row for _p, row in blocks_by_project]
        entry = {
            "blocks": len(blocks),
            "better": payload["better"],
            "per_project": dict(blocks_by_project),
            "wilcoxon_floor": stats.wilcoxon_floor(len(blocks)),
        }

        if payload["better"] == "higher":
            signed = [[-v for v in row] for row in blocks]
        else:
            signed = blocks

        stat, p, totals = stats.friedman(signed)
        corrected = stats.friedman_tie_correction(signed)
        entry["friedman"] = {
            "stat": stat,
            "stat_corrected": (stat / corrected) if (stat is not None and corrected) else None,
            "tie_factor": corrected,
            "p": p,
            "rank_totals": totals,
        }

        raw = []
        for a, b in PAIRS:
            ia, ib = STRATEGIES.index(a), STRATEGIES.index(b)
            xa = [row[ia] for row in signed]
            xb = [row[ib] for row in signed]
            _w, pv = stats.wilcoxon_signed_rank(xa, xb)
            ra = [row[ia] for row in blocks]
            rb = [row[ib] for row in blocks]
            effect = stats.rank_biserial(ra, rb)
            raw.append(("%s vs %s" % (a, b), pv, effect))

        adjusted = stats.holm([(label, pv) for label, pv, _d in raw])
        lookup = dict((label, adj) for label, _p, adj in adjusted)
        entry["pairwise"] = [
            {"comparison": label, "p": pv, "p_holm": lookup.get(label),
             "delta": delta,
             "magnitude": stats.rank_biserial_magnitude(delta),
             "wins": sum(1 for row in signed
                         if row[STRATEGIES.index(label.split()[0])]
                         < row[STRATEGIES.index(label.split()[2])])}
            for label, pv, delta in raw
        ]
        result[measure] = entry

    binary = []
    for project in projects:
        runs = grid.get((project, "S1"))
        if not runs:
            continue
        n = min(len(grid.get((project, s), [])) for s in STRATEGIES)
        for i in range(n):
            row = []
            for s in STRATEGIES:
                g = grid[(project, s)][i].get("grade") or {}
                row.append(1 if g.get("fully_successful") else 0)
            binary.append(row)
    if binary:
        q, p = stats.cochran_q(binary)
        result["accuracy"]["cochran_q_by_replicate"] = {
            "stat": q, "p": p, "blocks": len(binary),
            "caveat": "replicates are paired by index only, which is arbitrary "
                      "because runs within a cell are independent",
        }
    return result


def render(campaign, result, projects, loc):
    out = []
    w = out.append
    w("# Blocked analysis")
    w("")
    w("Campaign `%s`." % campaign)
    w("")
    w("Chapter 3 states the design is repeated-measures, every project being")
    w("processed under all three strategies, and prescribes the Friedman test")
    w("across the three conditions with the Wilcoxon signed-rank test for")
    w("pairwise statements and a correction for multiple comparisons. That is")
    w("what this file computes. The blocking unit is the project, so the number")
    w("of blocks is the number of projects, not the number of runs.")
    w("")
    w("Corpus: %s." % ", ".join("%s (%d LOC)" % (p, loc[p]) for p in projects))
    w("")

    for measure in ("accuracy", "cost_usd", "prompt_tokens"):
        entry = result.get(measure)
        if not entry:
            continue
        w("## %s" % measure.replace("_", " "))
        w("")
        w("Lower is better." if entry["better"] == "lower"
          else "Higher is better.")
        w("")
        w("| project | %s |" % " | ".join(STRATEGIES))
        w("|---|%s" % ("---|" * len(STRATEGIES)))
        spec = "%.5f" if measure == "cost_usd" else (
            "%.2f" if measure == "accuracy" else "%.0f")
        for project, row in entry["per_project"].items():
            w("| %s | %s |" % (project,
                               " | ".join(spec % v for v in row)))
        w("")
        f = entry["friedman"]
        w("Friedman over %d blocks: statistic %s, exact p %s." % (
            entry["blocks"],
            "-" if f["stat"] is None else "%.4f" % f["stat"],
            "-" if f["p"] is None else "%.4f" % f["p"]))
        w("")
        floor = entry["wilcoxon_floor"]
        w("Smallest p the pairwise test can attain with %d blocks: %s.%s" % (
            entry["blocks"],
            "-" if floor is None else "%.5f" % floor,
            "" if floor is None or floor < 0.05 else
            " **No pairwise result can be significant at 0.05 with this many "
            "blocks, whatever the effect size.**"))
        w("")
        w("| comparison | p | p after Holm | Cliff's delta | magnitude |")
        w("|---|---|---|---|---|")
        for row in entry["pairwise"]:
            w("| %s | %s | %s | %s | %s |" % (
                row["comparison"],
                "-" if row["p"] is None else "%.4f" % row["p"],
                "-" if row["p_holm"] is None else "%.4f" % row["p_holm"],
                "-" if row["delta"] is None else "%+.2f" % row["delta"],
                row["magnitude"]))
        w("")
        if measure == "accuracy" and "cochran_q_by_replicate" in entry:
            c = entry["cochran_q_by_replicate"]
            w("Cochran's Q on the binary outcome, %d blocks: statistic %s, "
              "p %s." % (c["blocks"],
                         "-" if c["stat"] is None else "%.4f" % c["stat"],
                         "-" if c["p"] is None else "%.6f" % c["p"]))
            w("")
            w("Caveat: %s." % c["caveat"])
            w("")
    return "\n".join(out) + "\n"


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if not argv:
        print("usage: python -m harness.blocked CAMPAIGN_DIR")
        return 2
    campaign = argv[0]
    path = os.path.join(campaign, "results.jsonl")
    records = [json.loads(line) for line in open(path, encoding="utf-8")
               if line.strip()]
    projects, loc, grid = cells(records)
    result = analyse(projects, grid)
    if "--json" in argv:
        print(json.dumps(result, indent=2))
    else:
        sys.stdout.write(render(campaign, result, projects, loc))
    return 0


if __name__ == "__main__":
    sys.exit(main())
