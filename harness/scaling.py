"""Controlled scaling family.

The four corpus projects differ in size and in everything else at once, so a
cost difference between them cannot be attributed to size. This builds a family
in which exactly one thing varies: a fixed target project is padded with
increasing quantities of code that compiles, is reachable by no call from the
active file, and is therefore irrelevant to the task. Whole-project
concatenation grows with the padding by construction, while a traversal seeded
in the active file should not reach it at all.
"""

import json
import os
import shutil
import sys

HEADER = """#ifndef PAD_%(tag)s_H
#define PAD_%(tag)s_H

namespace pad%(tag)s {

class Unit%(tag)s {
public:
    Unit%(tag)s();
    explicit Unit%(tag)s(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
"""

SOURCE = """#include "Pad%(tag)s.h"

namespace pad%(tag)s {

Unit%(tag)s::Unit%(tag)s() {
    state = %(tag)d;
}

Unit%(tag)s::Unit%(tag)s(int seed) {
    state = seed + %(tag)d;
}

int Unit%(tag)s::value() const {
    return state;
}

int Unit%(tag)s::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit%(tag)s::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit%(tag)s::reset() {
    state = %(tag)d;
}

}
"""

PAIR_LINES = len(HEADER.split("\n")) + len(SOURCE.split("\n"))


def base_loc(projdir):
    total = 0
    for name in sorted(os.listdir(projdir)):
        if not name.endswith((".cpp", ".h", ".hpp")):
            continue
        with open(os.path.join(projdir, name), encoding="utf-8") as fh:
            total += len([l for l in fh.read().split("\n") if l.strip()])
    return total


def build(source_project, target_dir, pad_lines):
    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    shutil.copytree(source_project, target_dir)
    written = 0
    tag = 0
    while written < pad_lines:
        tag += 1
        fields = {"tag": tag}
        with open(os.path.join(target_dir, "Pad%d.h" % tag), "w",
                  encoding="utf-8") as fh:
            fh.write(HEADER % fields)
        with open(os.path.join(target_dir, "Pad%d.cpp" % tag), "w",
                  encoding="utf-8") as fh:
            fh.write(SOURCE % fields)
        written += PAIR_LINES
    return tag


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)

    def opt(flag, default):
        return argv[argv.index(flag) + 1] if flag in argv else default

    if "--analyse" in argv:
        campaign = opt("--analyse", "")
        print(analyse(campaign))
        if "--figures" in argv:
            body, r = table(campaign)
            path = os.path.join(opt("--figures", "."), "ch5-tab-scaling.tex")
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(body)
            print("  wrote %s (Pearson r %+.4f)" % (path, r))
        return 0

    source = opt("--project", "corpus/geometry")
    active = opt("--active", "Canvas.cpp")
    out = opt("--out", "corpus-scaling")
    steps = [int(v) for v in opt("--pad", "0,250,500,1000,2000").split(",")]

    if os.path.exists(out):
        shutil.rmtree(out)
    os.makedirs(out)

    projects = []
    for pad in steps:
        name = "pad%04d" % pad
        target = os.path.join(out, name)
        units = build(source, target, pad)
        loc = base_loc(target)
        projects.append({"name": name, "dir": name, "active_file": active})
        print("  %-8s padding units %-3d total LOC %d" % (name, units, loc))

    with open(os.path.join(out, "manifest.json"), "w", encoding="utf-8") as fh:
        json.dump({"projects": projects}, fh, indent=2)
    print("wrote %s/manifest.json with %d variants" % (out, len(projects)))
    return 0



def analyse(campaign):
    """Report the padded family: does cost track size with topology fixed?"""
    import statistics
    from harness import stats
    from harness.report import load

    grid = {}
    for record in load(campaign):
        key = (record["project"], record["strategy"])
        entry = grid.setdefault(key, {"prompt": [], "cost": [], "ok": 0,
                                      "n": 0, "loc": record["project_metrics"]["loc"],
                                      "selected": None})
        call = record.get("model_call") or {}
        entry["prompt"].append(call.get("prompt_tokens"))
        entry["cost"].append(record.get("cost_usd"))
        entry["n"] += 1
        if (record.get("grade") or {}).get("fully_successful"):
            entry["ok"] += 1
        entry["selected"] = (record.get("context_meta") or {}).get("files_selected")

    variants = sorted({p for p, _s in grid}, key=lambda p: grid[(p, "S1")]["loc"])
    out = ["controlled scaling family: only project size varies\n\n"]
    out.append("  %-9s %5s %-5s %6s %8s %10s %9s %9s\n"
               % ("variant", "LOC", "str", "files", "prompt", "cost", "success",
                  "saving"))
    locs, savings, ratios = [], [], []
    for variant in variants:
        for strategy in ("S1", "S3"):
            cell = grid.get((variant, strategy))
            if not cell:
                continue
            prompt = statistics.mean([v for v in cell["prompt"] if v is not None])
            cost = statistics.mean([v for v in cell["cost"] if v is not None])
            saving = ""
            if strategy == "S3":
                base = grid[(variant, "S1")]
                bc = statistics.mean([v for v in base["cost"] if v is not None])
                bp = statistics.mean([v for v in base["prompt"] if v is not None])
                pct = 100.0 * (bc - cost) / bc if bc else 0.0
                saving = "%+7.1f%%" % pct
                locs.append(cell["loc"])
                savings.append(pct)
                ratios.append(100.0 * prompt / bp if bp else 0.0)
            out.append("  %-9s %5d %-5s %6s %8.0f %10.5f %9s %9s\n"
                       % (variant, cell["loc"], strategy, cell["selected"],
                          prompt, cost, "%d/%d" % (cell["ok"], cell["n"]),
                          saving))
    if len(locs) >= 3:
        out.append("\n  Pearson r of saving against LOC: %+.4f\n"
                   % stats.pearson(locs, savings))
        out.append("  S3 prompt as a percentage of S1: %s\n"
                   % ", ".join("%.1f" % r for r in ratios))
    return "".join(out)


def table(campaign):
    import statistics
    from harness import stats
    from harness.report import load

    grid = {}
    for record in load(campaign):
        key = (record["project"], record["strategy"])
        entry = grid.setdefault(key, {"prompt": [], "cost": [], "ok": 0, "n": 0,
                                      "loc": record["project_metrics"]["loc"],
                                      "files": record["project_metrics"]["files"],
                                      "selected": None})
        call = record.get("model_call") or {}
        entry["prompt"].append(call.get("prompt_tokens"))
        entry["cost"].append(record.get("cost_usd"))
        entry["n"] += 1
        if (record.get("grade") or {}).get("fully_successful"):
            entry["ok"] += 1
        entry["selected"] = (record.get("context_meta") or {}).get("files_selected")

    variants = sorted({p for p, _s in grid}, key=lambda p: grid[(p, "S1")]["loc"])
    rows, locs, savings = [], [], []
    for variant in variants:
        a, b = grid[(variant, "S1")], grid[(variant, "S3")]
        pa = statistics.mean([v for v in a["prompt"] if v is not None])
        pb = statistics.mean([v for v in b["prompt"] if v is not None])
        ca = statistics.mean([v for v in a["cost"] if v is not None])
        cb = statistics.mean([v for v in b["cost"] if v is not None])
        pct = 100.0 * (ca - cb) / ca if ca else 0.0
        locs.append(a["loc"])
        savings.append(pct)
        rows.append("%d & %d & %.0f & %.0f & %.1f & $%+.1f$ & %d/%d & %d/%d"
                    % (a["loc"], a["files"], pa, pb, 100.0 * pb / pa, pct,
                       a["ok"], a["n"], b["ok"], b["n"]))
    out = ["\\begin{table}[htbp]\n\\centering\n\\small\n",
           "\\begin{tabular}{rrrrrrcc}\n\\toprule\n",
           "& & \\multicolumn{3}{c}{Prompt tokens} & & "
           "\\multicolumn{2}{c}{Fully successful} \\\\\n",
           "\\cmidrule(l){3-5}\\cmidrule(l){7-8}\n",
           "LOC & Files & S1 & S3 & S3/S1 (\\%) & Saved (\\%) & S1 & S3 \\\\\n",
           "\\midrule\n"]
    for row in rows:
        out.append(row + " \\\\\n")
    out.append("\\bottomrule\n\\end{tabular}\n")
    out.append("\\caption{\\CaptionScaling}\n\\label{tab:scaling}\n\\end{table}\n")
    return "".join(out), stats.pearson(locs, savings)


if __name__ == "__main__":
    sys.exit(main())
