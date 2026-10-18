import math
import os
import sys

from harness import blocked, stats
from harness.dashboard import (STRATEGIES, STRATEGY_LABEL, _figure, _table,
                               build_cells, write_captions)
from harness.report import load

ARM_KEYS = ("default", "suppressed")

ARM_LABEL = {
    "default": "default reasoning",
    "suppressed": "suppressed reasoning",
}

ARM_SHORT = {"default": "default", "suppressed": "suppressed"}

ARM_DASH = {"default": "solid", "suppressed": "densely dashed"}

BAR_FILL = {"S1": "black!72", "S2": "black!38", "S3": "black!10"}

BOX_STYLE = {"S1": "draw=black, fill=black!72",
             "S2": "draw=black, fill=black!38",
             "S3": "draw=black, fill=black!10"}

PANEL_BOX = (
    "    width=7.1cm, height=5.6cm,\n"
    "    font=\\small,\n"
    "    boxplot/draw direction=y,\n"
    "    ymajorgrids=true,\n"
    "    grid style={black!15},\n"
    "    tick label style={font=\\small},\n"
    "    label style={font=\\small},\n"
    "    title style={font=\\small},\n"
    "    xtick={1,2,3,4},\n"
    "    x tick label style={rotate=30, anchor=east},\n"
    "    ymin=0,\n"
)

MARK = {"S1": "*", "S2": "square*", "S3": "triangle*"}

STRAT_DASH = {"S1": "solid", "S2": "densely dashed", "S3": "densely dotted"}

PANEL = (
    "    width=7.1cm, height=5.6cm,\n"
    "    font=\\small,\n"
    "    ymajorgrids=true,\n"
    "    grid style={black!15},\n"
    "    tick label style={font=\\scriptsize},\n"
    "    label style={font=\\small},\n"
    "    title style={font=\\small},\n"
    "    ybar, bar width=6pt,\n"
    "    ymin=0, ymax=148, ytick={0,25,50,75,100},\n"
    "    enlarge x limits=0.22,\n"
    "    xtick=data,\n"
    "    x tick label style={rotate=30, anchor=east},\n"
)

WIDE = (
    "    width=12cm, height=6.2cm,\n"
    "    font=\\small,\n"
    "    legend cell align=left,\n"
    "    legend style={draw=black!30, fill=white, font=\\scriptsize},\n"
    "    ymajorgrids=true,\n"
    "    grid style={black!15},\n"
    "    tick label style={font=\\small},\n"
    "    label style={font=\\small},\n"
)

EXTRA_CAPTIONS = (
    ("CaptionSaving",
     "how the cost saving of S3 relative to S1 changes with project size"),
    ("CaptionSavingTable",
     "the per-project prompt ratio and cost saving in both arms"),
)


def fmt(value, spec="%.4f", dash="---"):
    return dash if value is None else spec % value


def mfmt(value, spec="%+.1f", dash="---"):
    return dash if value is None else "$" + (spec % value) + "$"


def saving_interval(grid, project):
    s1 = grid.get((project, "S1"))
    s3 = grid.get((project, "S3"))
    if not s1 or not s3:
        return None, None
    return stats.bootstrap_saving_ci(s1["cost_values"], s3["cost_values"])


def load_arms(paths):
    arms = {}
    for key in ARM_KEYS:
        records = load(paths[key])
        if not records:
            raise SystemExit("no measured runs in %s" % paths[key])
        projects, loc, grid = build_cells(records)
        bprojects, bloc, bgrid = blocked.cells(records)
        arms[key] = {
            "dir": paths[key],
            "records": records,
            "projects": projects,
            "loc": loc,
            "grid": grid,
            "blocked": blocked.analyse(bprojects, bgrid),
            "blocked_projects": bprojects,
        }
    return arms


def consistency(arms):
    out = []
    a, b = arms["default"], arms["suppressed"]
    if a["projects"] != b["projects"]:
        out.append("the two arms do not cover the same projects: %s against %s"
                   % (a["projects"], b["projects"]))
        return out
    for project in a["projects"]:
        if a["loc"][project] != b["loc"][project]:
            out.append("%s has a different size in the two arms" % project)
        for strategy in STRATEGIES:
            ca, cb = a["grid"].get((project, strategy)), b["grid"].get(
                (project, strategy))
            if ca is None or cb is None:
                out.append("%s %s is missing from one arm" % (project, strategy))
                continue
            if ca["n"] != cb["n"]:
                out.append("%s %s has %d runs in one arm and %d in the other"
                           % (project, strategy, ca["n"], cb["n"]))
            if ca["prompt"]["mean"] != cb["prompt"]["mean"]:
                out.append("%s %s sent a different prompt in the two arms, "
                           "%s against %s"
                           % (project, strategy, ca["prompt"]["mean"],
                              cb["prompt"]["mean"]))
            if ca["selected"] != cb["selected"]:
                out.append("%s %s selected a different file count in the "
                           "two arms" % (project, strategy))
    return out


def saving(grid, project):
    s1 = grid.get((project, "S1"))
    s3 = grid.get((project, "S3"))
    if not s1 or not s3 or not s1["cost"]["mean"]:
        return None
    return 100.0 * (1.0 - s3["cost"]["mean"] / s1["cost"]["mean"])


def prompt_ratio(grid, project):
    s1 = grid.get((project, "S1"))
    s3 = grid.get((project, "S3"))
    if not s1 or not s3 or not s1["prompt"]["mean"]:
        return None
    return 100.0 * s3["prompt"]["mean"] / s1["prompt"]["mean"]


def figure_accuracy(projects, arms):
    symbolic = "    symbolic x coords={" + ",".join(projects) + "},\n"
    body = ["\\begin{tikzpicture}\n\\begin{groupplot}[\n",
            "    group style={group size=2 by 1, horizontal sep=1.5cm},\n]\n"]
    for index, key in enumerate(ARM_KEYS):
        head = [PANEL, symbolic, "    title={" + ARM_LABEL[key] + "},\n"]
        if index == 0:
            head.append("    ylabel={Fully successful runs (per cent)},\n")
            head.append("    legend style={draw=black!30, fill=white, "
                        "font=\\scriptsize, at={(0.98,0.98)}, "
                        "anchor=north east, legend columns=3},\n")
            head.append("    legend cell align=left,\n")
        body.append("\\nextgroupplot[\n" + "".join(head) + "]\n")
        grid = arms[key]["grid"]
        for strategy in STRATEGIES:
            pts = []
            for project in projects:
                c = grid.get((project, strategy))
                if c:
                    pts.append("(%s,%.1f)"
                               % (project, 100.0 * c["ok"] / c["n"]))
            body.append("\\addplot[draw=black, fill=%s] coordinates {%s};\n"
                        % (BAR_FILL[strategy], " ".join(pts)))
            if index == 0:
                body.append("\\addlegendentry{%s}\n" % strategy)
    body.append("\\end{groupplot}\n\\end{tikzpicture}\n")
    return _figure("fig:accuracy", "".join(body), "CaptionAccuracy")


def figure_cost(projects, loc, arms):
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", WIDE,
            "    ylabel={Mean cost per run (USD cents)},\n",
            "    xlabel={Project size (lines of code)},\n",
            "    legend style={draw=black!30, fill=white, font=\\scriptsize, "
            "at={(0.98,0.98)}, anchor=north east, legend columns=3},\n",
            "    ytick distance=0.5,\n",
            "    enlarge y limits={upper, value=0.45},\n",
            "    xmin=0, ymin=0,\n]\n"]
    for key in ARM_KEYS:
        grid = arms[key]["grid"]
        for strategy in STRATEGIES:
            pts = []
            for project in projects:
                c = grid.get((project, strategy))
                if c and c["cost"]["mean"] is not None:
                    pts.append("(%d,%.4f)"
                               % (loc[project], 100.0 * c["cost"]["mean"]))
            body.append("\\addplot[mark=%s, mark size=2pt, %s] "
                        "coordinates {%s};\n"
                        % (MARK[strategy], ARM_DASH[key], " ".join(pts)))
            body.append("\\addlegendentry{%s, %s}\n"
                        % (strategy, ARM_SHORT[key]))
    body.append("\\end{axis}\n\\end{tikzpicture}\n")
    return _figure("fig:cost", "".join(body), "CaptionCost")


def figure_prompt(projects, loc, grid):
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", WIDE,
            "    ylabel={Mean prompt tokens},\n",
            "    xlabel={Project size (lines of code)},\n",
            "    legend pos=north east,\n",
            "    enlarge y limits={upper, value=0.5},\n",
            "    xmin=0, ymin=0,\n]\n"]
    for strategy in STRATEGIES:
        pts = []
        for project in projects:
            c = grid.get((project, strategy))
            if c and c["prompt"]["mean"] is not None:
                pts.append("(%d,%.1f)" % (loc[project], c["prompt"]["mean"]))
        body.append("\\addplot[mark=%s, mark size=2pt, %s] "
                    "coordinates {%s};\n"
                    % (MARK[strategy], STRAT_DASH[strategy], " ".join(pts)))
        body.append("\\addlegendentry{%s %s}\n"
                    % (strategy, STRATEGY_LABEL[strategy]))
    body.append("\\end{axis}\n\\end{tikzpicture}\n")
    return _figure("fig:promptscaling", "".join(body), "CaptionPromptScaling")


def figure_ratio(projects, loc, grid):
    pts = []
    for project in projects:
        value = prompt_ratio(grid, project)
        if value is not None:
            pts.append("(%d,%.1f)" % (loc[project], value))
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", WIDE,
            "    ylabel={S3 prompt as a percentage of S1},\n",
            "    xlabel={Project size (lines of code)},\n",
            "    legend pos=north east,\n",
            "    xmin=0, ymin=0, ymax=115, ytick={0,25,50,75,100},\n]\n",
            "\\addplot[mark=triangle*, mark size=2.5pt, densely dotted] "
            "coordinates {" + " ".join(pts) + "};\n",
            "\\addlegendentry{S3 relative to S1}\n",
            "\\end{axis}\n\\end{tikzpicture}\n"]
    return _figure("fig:ratio", "".join(body), "CaptionRatio")


def figure_saving(projects, loc, arms):
    series = {}
    values = []
    for key in ARM_KEYS:
        grid = arms[key]["grid"]
        pts = []
        for project in projects:
            value = saving(grid, project)
            if value is not None:
                pts.append("(%d,%.2f)" % (loc[project], value))
                values.append(value)
        series[key] = pts
    floor_y = int(min(values)) - 4 if values else 0
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", WIDE,
            "    ylabel={Cost saved by S3 against S1 (per cent)},\n",
            "    xlabel={Project size (lines of code)},\n",
            "    legend pos=north east,\n",
            "    enlarge y limits={upper, value=0.45},\n",
            "    xmin=0, ymin=%d,\n]\n" % floor_y,
            "\\draw[black!45, densely dotted] "
            "({rel axis cs:0,0}|-{axis cs:0,0}) -- "
            "({rel axis cs:1,0}|-{axis cs:0,0});\n"]
    for key in ARM_KEYS:
        body.append("\\addplot[mark=triangle*, mark size=2.5pt, %s] "
                    "coordinates {%s};\n" % (ARM_DASH[key], " ".join(series[key])))
        body.append("\\addlegendentry{%s}\n" % ARM_LABEL[key])
    body.append("\\end{axis}\n\\end{tikzpicture}\n")
    return _figure("fig:saving", "".join(body), "CaptionSaving")


def table_accuracy(projects, loc, arms):
    rows = []
    for i, project in enumerate(projects):
        if i:
            rows.append(None)
        first = True
        for strategy in STRATEGIES:
            ca = arms["default"]["grid"].get((project, strategy))
            cb = arms["suppressed"]["grid"].get((project, strategy))
            if not ca or not cb:
                continue
            rows.append("%s & %s & %s & %s & %d & %d/%d & %.0f & %d/%d & %.0f"
                        % (project if first else "",
                           str(loc[project]) if first else "",
                           str(ca["files"]) if first else "",
                           strategy, ca["selected"] or 0,
                           ca["ok"], ca["n"], 100.0 * ca["ok"] / ca["n"],
                           cb["ok"], cb["n"], 100.0 * cb["ok"] / cb["n"]))
            first = False
    totals = []
    for key in ARM_KEYS:
        grid = arms[key]["grid"]
        for strategy in STRATEGIES:
            ok = sum(grid[(p, strategy)]["ok"] for p in projects
                     if (p, strategy) in grid)
            n = sum(grid[(p, strategy)]["n"] for p in projects
                    if (p, strategy) in grid)
            totals.append((key, strategy, ok, n))
    rows.append(None)
    first = True
    for strategy in STRATEGIES:
        a = [t for t in totals if t[0] == "default" and t[1] == strategy][0]
        b = [t for t in totals if t[0] == "suppressed" and t[1] == strategy][0]
        rows.append("%s & %s & %s & %s & --- & %d/%d & %.0f & %d/%d & %.0f"
                    % ("All" if first else "",
                       str(sum(loc[p] for p in projects)) if first else "",
                       str(sum(arms["default"]["grid"][(p, "S1")]["files"]
                               for p in projects)) if first else "",
                       strategy, a[2], a[3], 100.0 * a[2] / a[3],
                       b[2], b[3], 100.0 * b[2] / b[3]))
        first = False
    return _table(
        "tab:accuracy", "CaptionAccuracyTable", "lrrlrrrrr",
        "& & & & & \\multicolumn{2}{c}{Default reasoning} & "
        "\\multicolumn{2}{c}{Suppressed} \\\\\n"
        "\\cmidrule(l){6-7}\\cmidrule(l){8-9}\n"
        "Project & LOC & Files & Str. & Sel. & Succ. & \\% & Succ. & \\%",
        rows)


def table_cost(projects, arms):
    rows = []
    for i, project in enumerate(projects):
        if i:
            rows.append(None)
        first = True
        for strategy in STRATEGIES:
            ca = arms["default"]["grid"].get((project, strategy))
            cb = arms["suppressed"]["grid"].get((project, strategy))
            if not ca or not cb:
                continue
            rows.append("%s & %s & %.0f & %.0f & %.0f (%.0f) & %.4f "
                        "& %.0f & %.4f"
                        % (project if first else "", strategy,
                           ca["prompt"]["mean"] or 0.0,
                           ca["output"]["mean"] or 0.0,
                           ca["think"]["mean"] or 0.0,
                           ca["think"]["sd"] or 0.0,
                           ca["cost"]["mean"] or 0.0,
                           cb["output"]["mean"] or 0.0,
                           cb["cost"]["mean"] or 0.0))
            first = False
    return _table(
        "tab:cost", "CaptionCostTable", "llrrrrrr",
        "& & & \\multicolumn{3}{c}{Default reasoning} & "
        "\\multicolumn{2}{c}{Suppressed} \\\\\n"
        "\\cmidrule(l){4-6}\\cmidrule(l){7-8}\n"
        "Project & Str. & Prompt & Output & Think (sd) & Cost & Output & Cost",
        rows)


def table_saving(projects, loc, arms):
    rows = []
    ratios, sav_a, sav_b, sizes = [], [], [], []
    for project in projects:
        ratio = prompt_ratio(arms["suppressed"]["grid"], project)
        a = saving(arms["default"]["grid"], project)
        b = saving(arms["suppressed"]["grid"], project)
        alo, ahi = saving_interval(arms["default"]["grid"], project)
        blo, bhi = saving_interval(arms["suppressed"]["grid"], project)
        rows.append("%s & %d & %s & %s & %s & %s & %s"
                    % (project, loc[project], fmt(ratio, "%.1f"),
                       mfmt(a), _interval(alo, ahi),
                       mfmt(b), _interval(blo, bhi)))
        if None not in (ratio, a, b):
            sizes.append(loc[project])
            ratios.append(ratio)
            sav_a.append(a)
            sav_b.append(b)
    rows.append(None)
    rows.append("Pearson $r$ vs LOC & --- & %s & %s & --- & %s & ---"
                % (mfmt(stats.pearson(sizes, ratios), "%+.4f"),
                   mfmt(stats.pearson(sizes, sav_a), "%+.4f"),
                   mfmt(stats.pearson(sizes, sav_b), "%+.4f")))
    return _tight_table(
        "tab:saving", "CaptionSavingTable", "lrrrlrl",
        "& & & \\multicolumn{2}{c}{Default reasoning} & "
        "\\multicolumn{2}{c}{Suppressed reasoning} \\\\\n"
        "\\cmidrule(l){4-5}\\cmidrule(l){6-7}\n"
        "Project & LOC & Prompt (\\%) & Saved (\\%) & 95\\% CI & "
        "Saved (\\%) & 95\\% CI",
        rows)


def _interval(lo, hi):
    if lo is None or hi is None:
        return "---"
    return "$[%+.1f,\\,%+.1f]$" % (lo, hi)


def _tight_table(label, macro, spec, header, rows):
    out = ["\\begin{table}[htbp]\n\\centering\n\\footnotesize\n",
           "\\begin{tabular}{" + spec + "}\n\\toprule\n",
           header + " \\\\\n\\midrule\n"]
    for row in rows:
        if row is None:
            out.append("\\midrule\n")
        else:
            out.append(row + " \\\\\n")
    out.append("\\bottomrule\n\\end{tabular}\n")
    out.append("\\caption{\\" + macro + "}\n\\label{" + label + "}\n")
    out.append("\\end{table}\n")
    return "".join(out)



def figure_distribution(projects, arms):
    ceiling = 0.0
    for key in ARM_KEYS:
        for cellv in arms[key]["grid"].values():
            values = [100.0 * v for v in cellv["cost_values"] if v is not None]
            if values:
                ceiling = max(ceiling, max(values))
    ceiling = math.ceil(ceiling * 10.0) / 10.0 + 0.1

    panels = []
    for idx, key in enumerate(ARM_KEYS):
        grid = arms[key]["grid"]
        body = ["\\nextgroupplot[\n", PANEL_BOX,
                "    ymax=%.1f,\n" % ceiling,
                "    title={%s},\n" % ARM_LABEL[key],
                "    xticklabels={" + ",".join(projects) + "},\n"]
        if idx == 0:
            body.append("    ylabel={Cost per run (USD cents)},\n")
        body.append("]\n")
        for strategy in STRATEGIES:
            offset = {"S1": -0.22, "S2": 0.0, "S3": 0.22}[strategy]
            for project in projects:
                cellv = grid.get((project, strategy))
                if not cellv:
                    continue
                q = stats.quartiles([100.0 * v for v in cellv["cost_values"]
                                     if v is not None])
                if not q:
                    continue
                body.append(
                    "\\addplot[boxplot prepared={draw position=%.2f, "
                    "lower whisker=%.3f, lower quartile=%.3f, median=%.3f, "
                    "upper quartile=%.3f, upper whisker=%.3f, box extend=0.18}, "
                    "%s, solid] coordinates {%s};\n"
                    % (projects.index(project) + 1 + offset,
                       q["min"], q["q1"], q["median"], q["q3"], q["max"],
                       BOX_STYLE[strategy],
                       " ".join("(0,%.3f)" % o for o in q["outliers"])))
        panels.append("".join(body))
    return ("\\begin{figure}[htbp]\n\\centering\n\\begin{tikzpicture}\n"
            "\\begin{groupplot}[\n    group style={group size=2 by 1, "
            "horizontal sep=1.5cm},\n]\n"
            + "".join(panels)
            + "\\end{groupplot}\n\\end{tikzpicture}\n"
              "\\caption{\\CaptionDistribution}\n"
              "\\label{fig:distribution}\n\\end{figure}\n")


def table_stats(arms):
    rows = []
    measures = (("accuracy", "Accuracy", ARM_KEYS),
                ("cost_usd", "Cost", ARM_KEYS),
                ("prompt_tokens", "Prompt tokens", ("suppressed",)))
    first_block = True
    for measure, label, keys in measures:
        if not first_block:
            rows.append(None)
        first_block = False
        for key in keys:
            entry = arms[key]["blocked"].get(measure)
            if not entry:
                continue
            scope = "both" if len(keys) == 1 else ARM_SHORT[key]
            f = entry["friedman"]
            rows.append("%s & %s & Friedman, $b=%d$ & %s & %s & %s & --- & ---"
                        % (label, scope, entry["blocks"],
                           fmt(f["stat"], "%.3f"),
                           fmt(f.get("stat_corrected"), "%.3f"),
                           fmt(f["p"], "%.4f")))
            for row in entry["pairwise"]:
                rows.append("& & %s & --- & --- & %s & %s & %s (%s)"
                            % (row["comparison"], fmt(row["p"], "%.4f"),
                               fmt(row["p_holm"], "%.4f"),
                               mfmt(row["delta"], "%+.2f"), row["magnitude"]))
    return _tight_table(
        "tab:stats", "CaptionStatsTable", "lllrrrrl",
        "Quantity & Arm & Test or comparison & Stat. & Tie-corr. & $p$ & "
        "$p_{\\mathrm{Holm}}$ & $r_{\\mathrm{rb}}$",
        rows)


def append_captions(target):
    path = os.path.join(target, "captions.tex")
    existing = ""
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as fh:
            existing = fh.read()
    added = []
    lines = []
    for macro, todo in EXTRA_CAPTIONS:
        if ("\\newcommand{\\%s}" % macro) in existing:
            continue
        lines.append("\\newcommand{\\%s}{\\textcolor{red}{TODO (Umair): %s}}\n"
                     % (macro, todo))
        added.append(macro)
    if lines:
        with open(path, "a", encoding="utf-8") as fh:
            fh.write("".join(lines))
    return added


def write(target, arms):
    if not os.path.isdir(target):
        os.makedirs(target)
    projects = arms["default"]["projects"]
    loc = arms["default"]["loc"]
    prompt_grid = arms["suppressed"]["grid"]
    files = {
        "ch5-accuracy.tex": figure_accuracy(projects, arms),
        "ch5-cost.tex": figure_cost(projects, loc, arms),
        "ch5-saving.tex": figure_saving(projects, loc, arms),
        "ch5-prompt-scaling.tex": figure_prompt(projects, loc, prompt_grid),
        "ch5-ratio.tex": figure_ratio(projects, loc, prompt_grid),
        "ch5-distribution.tex": figure_distribution(projects, arms),
        "ch5-tab-accuracy.tex": table_accuracy(projects, loc, arms),
        "ch5-tab-cost.tex": table_cost(projects, arms),
        "ch5-tab-saving.tex": table_saving(projects, loc, arms),
        "ch5-tab-stats.tex": table_stats(arms),
    }
    written = []
    for name, content in sorted(files.items()):
        path = os.path.join(target, name)
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(content)
        written.append(path)
    caption_path, created = write_captions(target)
    added = append_captions(target)
    note = "created" if created else (
        "appended %s" % ", ".join(added) if added else "kept unchanged")
    written.append("%s  (%s)" % (caption_path, note))
    return written


def summarise(arms):
    out = []
    w = out.append
    projects = arms["default"]["projects"]
    loc = arms["default"]["loc"]
    w("two-arm figure and table generation")
    for key in ARM_KEYS:
        n = len(arms[key]["records"])
        w("  %-11s %s  %d runs" % (ARM_LABEL[key], arms[key]["dir"], n))
    problems = consistency(arms)
    if problems:
        w("  consistency problems:")
        for p in problems:
            w("    %s" % p)
    else:
        w("  the two arms agree on corpus, run counts, prompt sizes and "
          "selection")
    w("")
    w("  %-11s %6s %8s %8s %9s %9s"
      % ("project", "loc", "acc def", "acc sup", "save def", "save sup"))
    for project in projects:
        a = arms["default"]["grid"]
        b = arms["suppressed"]["grid"]
        w("  %-11s %6d %7.0f%% %7.0f%% %8s%% %8s%%"
          % (project, loc[project],
             100.0 * a[(project, "S3")]["ok"] / a[(project, "S3")]["n"],
             100.0 * b[(project, "S3")]["ok"] / b[(project, "S3")]["n"],
             fmt(saving(a, project), "%+.1f"),
             fmt(saving(b, project), "%+.1f")))
    w("")
    subset = [p for p in projects
              if (arms["default"]["grid"][(p, "S3")]["selected"] or 0)
              < arms["default"]["grid"][(p, "S3")]["files"]]
    if len(subset) >= 3:
        sizes = [loc[p] for p in subset]
        for key in ARM_KEYS:
            values = [saving(arms[key]["grid"], p) for p in subset]
            w("  %-11s saving against LOC over strict-subset projects (%s): "
              "Pearson r %s"
              % (ARM_SHORT[key], ", ".join(subset),
                 fmt(stats.pearson(sizes, values), "%+.4f")))
    for key in ARM_KEYS:
        pts = [(loc[p], saving(arms[key]["grid"], p)) for p in projects]
        crossings = []
        for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
            if y1 is None or y2 is None:
                continue
            if (y1 < 0.0 <= y2) or (y2 < 0.0 <= y1):
                crossings.append(x1 + (0.0 - y1) / (y2 - y1) * (x2 - x1))
        if crossings:
            w("  %-11s saving crosses zero %s, linear interpolation %s LOC"
              % (ARM_SHORT[key],
                 "once" if len(crossings) == 1 else "%d times" % len(crossings),
                 ", ".join("%.0f" % c for c in crossings)))
        else:
            w("  %-11s saving does not cross zero within the corpus"
              % ARM_SHORT[key])
    w("")
    for key in ARM_KEYS:
        for measure in ("accuracy", "cost_usd", "prompt_tokens"):
            entry = arms[key]["blocked"].get(measure)
            if not entry:
                continue
            f = entry["friedman"]
            w("  %-11s %-14s Friedman stat %s p %s   pairwise floor %s"
              % (ARM_SHORT[key], measure, fmt(f["stat"], "%.4f"),
                 fmt(f["p"], "%.4f"), fmt(entry["wilcoxon_floor"], "%.5f")))
    return "\n".join(out)


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    paths = {}
    target = None
    for key in ARM_KEYS:
        flag = "--" + key
        if flag in argv:
            paths[key] = argv[argv.index(flag) + 1]
    if "--figures" in argv:
        target = argv[argv.index("--figures") + 1]
    if len(paths) != len(ARM_KEYS):
        print("usage: python -m harness.chapter5 --default DIR "
              "--suppressed DIR [--figures DIR]")
        return 2
    arms = load_arms(paths)
    print(summarise(arms))
    if target:
        print("")
        for path in write(target, arms):
            print("figure     %s" % path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
