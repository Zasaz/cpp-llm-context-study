import datetime
import html
import json
import os
import sys

from harness import stats
from harness.report import load, thought_tokens


STRATEGIES = ("S1", "S2", "S3")

STRATEGY_LABEL = {
    "S1": "whole project",
    "S2": "active file only",
    "S3": "structural selection",
}

STATUS_ORDER = {"BLOCKED": 0, "PARTIAL": 1, "COMPLETE": 2, "NOT DATA DEPENDENT": 3}


def campaign_meta(campaign_dir):
    path = os.path.join(campaign_dir, "summary.json")
    if not os.path.exists(path):
        return {}
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def project_order(records):
    loc = {}
    for record in records:
        loc[record["project"]] = record["project_metrics"]["loc"]
    return sorted(loc, key=lambda p: loc[p]), loc


def cell(records, project, strategy):
    runs = [r for r in records
            if r["project"] == project and r["strategy"] == strategy]
    if not runs:
        return None
    grades = [r.get("grade") or {} for r in runs]
    calls = [r.get("model_call") or {} for r in runs]
    metrics = runs[0]["project_metrics"]
    meta = runs[0].get("context_meta") or {}
    return {
        "project": project,
        "strategy": strategy,
        "n": len(runs),
        "ok": sum(1 for g in grades if g.get("fully_successful")),
        "loc": metrics["loc"],
        "files": metrics["files"],
        "selected": meta.get("files_selected"),
        "prompt": stats.summarise([c.get("prompt_tokens") for c in calls]),
        "output": stats.summarise([c.get("output_tokens") for c in calls]),
        "think": stats.summarise([thought_tokens(c) for c in calls]),
        "cost": stats.summarise([r.get("cost_usd") for r in runs]),
        "cost_values": [r.get("cost_usd") for r in runs],
        "prompt_values": [c.get("prompt_tokens") for c in calls],
        "grades": grades,
    }


def build_cells(records):
    projects, loc = project_order(records)
    grid = {}
    for project in projects:
        for strategy in STRATEGIES:
            c = cell(records, project, strategy)
            if c:
                grid[(project, strategy)] = c
    return projects, loc, grid


def accuracy_totals(grid, projects):
    totals = {}
    for strategy in STRATEGIES:
        ok = sum(grid[(p, strategy)]["ok"] for p in projects if (p, strategy) in grid)
        n = sum(grid[(p, strategy)]["n"] for p in projects if (p, strategy) in grid)
        totals[strategy] = (ok, n)
    return totals


def accuracy_tests(totals):
    rows = []
    for a, b in (("S1", "S2"), ("S1", "S3"), ("S2", "S3")):
        ok_a, n_a = totals[a]
        ok_b, n_b = totals[b]
        p = fisher = stats.fisher_exact(ok_a, n_a - ok_a, ok_b, n_b - ok_b)
        rows.append({
            "pair": "%s vs %s" % (a, b),
            "a": "%d/%d" % (ok_a, n_a),
            "b": "%d/%d" % (ok_b, n_b),
            "p": p,
            "significant": p < 0.05,
        })
    return rows


def paired_tests(grid, projects, key):
    rows = []
    for project in projects:
        for a, b in (("S1", "S3"), ("S1", "S2"), ("S2", "S3")):
            ca, cb = grid.get((project, a)), grid.get((project, b))
            if not ca or not cb:
                continue
            xa = [v for v in ca[key] if isinstance(v, (int, float))]
            xb = [v for v in cb[key] if isinstance(v, (int, float))]
            if len(xa) < 2 or len(xb) < 2:
                continue
            _u, p = stats.mann_whitney(xa, xb)
            delta = stats.cliffs_delta(xa, xb)
            rows.append({
                "project": project,
                "pair": "%s vs %s" % (a, b),
                "mean_a": sum(xa) / len(xa),
                "mean_b": sum(xb) / len(xb),
                "p": p,
                "delta": delta,
                "magnitude": stats.cliffs_magnitude(delta),
                "significant": p is not None and p < 0.05,
            })
    return rows


def failure_modes(records):
    kinds = {}
    for record in records:
        g = record.get("grade") or {}
        if g.get("fully_successful"):
            continue
        if g.get("refused"):
            kind = "refused"
        elif not g.get("compiled"):
            kind = "did not compile"
        else:
            kind = g.get("failure_kind") or "other"
        key = (record["strategy"], kind)
        kinds[key] = kinds.get(key, 0) + 1
    return sorted(kinds.items())


def degenerate_conditions(grid, projects):
    bad = []
    for project in projects:
        s1, s3 = grid.get((project, "S1")), grid.get((project, "S3"))
        if s1 and s3 and s1["selected"] == s3["selected"]:
            bad.append(project)
    return bad


def evidence_map(records, grid, projects, loc, meta, totals, acc_tests):
    n_per_cell = min(c["n"] for c in grid.values()) if grid else 0
    degenerate = degenerate_conditions(grid, projects)
    sizes = sorted(set(loc.values()))
    have_data = len(records) > 0
    any_sig_accuracy = any(r["significant"] for r in acc_tests)
    pricing = meta.get("pricing") or {}

    def req(label, ok, detail=""):
        return {"label": label, "ok": bool(ok), "detail": detail}

    items = []

    items.append({
        "id": "ch5-opening",
        "where": "chapters/05-evaluation.tex:4",
        "title": "Chapter opening still says no measurement exists",
        "pages": "",
        "action": "Delete this TODO. It is now false.",
        "requirements": [
            req("a campaign with measured runs exists", have_data,
                "%d runs in %s" % (len(records), os.path.basename(meta.get("_dir", "")))),
        ],
    })

    items.append({
        "id": "ch5-1",
        "where": "chapters/05-evaluation.tex:11",
        "title": "5.1 Experimental Setup",
        "pages": "approx. 4 pages",
        "action": "Writable now.",
        "requirements": [
            req("corpus of at least three projects", len(projects) >= 3,
                "%d projects: %s" % (len(projects), ", ".join(projects))),
            req("documented size range", len(sizes) >= 3,
                "%d to %d LOC" % (min(sizes), max(sizes)) if sizes else ""),
            req("model version recorded", bool(pricing.get("model")),
                str(pricing.get("model"))),
            req("generation parameters recorded", True,
                "temperature 1.0, top_p 0.95, candidate_count 1"),
            req("pricing basis recorded and dated", bool(pricing.get("read_on")),
                "%s / %s USD per million, read %s"
                % (pricing.get("input_per_million_usd"),
                   pricing.get("output_per_million_usd"), pricing.get("read_on"))),
            req("package versions of google-genai pinned in the record", False,
                "not captured by the harness, must be stated by hand"),
        ],
    })

    items.append({
        "id": "ch5-2",
        "where": "chapters/05-evaluation.tex:18",
        "title": "5.2 Accuracy and Cost (RQ1, RQ2)",
        "pages": "approx. 4 pages",
        "action": "Writable now.",
        "requirements": [
            req("success rate per condition", True,
                "  ".join("%s %d/%d" % (s, totals[s][0], totals[s][1])
                          for s in STRATEGIES)),
            req("token counts per condition", True, "prompt, output and thinking"),
            req("cost per condition", True, "derived from the dated price list"),
            req("failure modes classified", True,
                "compile, assertion, crash, refusal separated"),
            req("significance test for the accuracy difference", any_sig_accuracy,
                "Fisher exact, pairwise"),
            req("diagrams", True, "figures/ch5-accuracy.tex, figures/ch5-cost.tex"),
        ],
    })

    items.append({
        "id": "ch5-3",
        "where": "chapters/05-evaluation.tex:24",
        "title": "5.3 Scaling and the Crossover Point (RQ3)",
        "pages": "approx. 4 pages",
        "action": "Writable, but state the limits explicitly.",
        "requirements": [
            req("at least four size points", len(sizes) >= 4,
                ", ".join(str(s) for s in sizes)),
            req("size points well spread rather than clustered", False,
                "two projects near 150 LOC, then 598 and 1203, so effectively "
                "three scales not four"),
            req("every project separates S1 from S3", not degenerate,
                ("degenerate: " + ", ".join(degenerate)) if degenerate
                else "all projects separate"),
            req("inference statistics", True, "Mann-Whitney exact, Cliff's delta"),
            req("adequate repetitions per condition", n_per_cell >= 10,
                "n = %d per condition, underpowered for cost" % n_per_cell),
        ],
    })

    items.append({
        "id": "ch5-4",
        "where": "chapters/05-evaluation.tex:31",
        "title": "5.4 Discussion and Threats to Validity",
        "pages": "approx. 3 pages",
        "action": "Blocked on one decision.",
        "requirements": [
            req("degenerate condition disclosed", bool(degenerate),
                "inventory selects the same files under S1 and S3"),
            req("S3 selection ratio declared as designed, not observed", False,
                "S3 lands on exactly 50 percent in scheduler and telemetry "
                "because the clusters were built to separate"),
            req("corpus provenance disclosed", False,
                "all four projects are synthetic and author-written"),
            req("include-stripping conflict resolved", False,
                "Chapter 4 documents the opposite of what the code now does"),
            req("extraction fidelity limitation stated", False,
                "lexical extraction, no preprocessing, extractor 2.1-typed"),
        ],
    })

    items.append({
        "id": "ch6",
        "where": "chapters/06-conclusion.tex:4",
        "title": "Chapter 6 Conclusion",
        "pages": "exactly 1 page, no sections",
        "action": "Depends on 5.4 being written first.",
        "requirements": [
            req("Chapter 5 complete", False, "5.4 is blocked"),
        ],
    })

    items.append({
        "id": "abstract",
        "where": "frontmatter/abstract.tex",
        "title": "Abstract",
        "pages": "",
        "action": "Written last, from results that exist.",
        "requirements": [
            req("headline result available", True,
                "S2 %d/%d, S1 %d/%d, S3 %d/%d"
                % (totals["S2"][0], totals["S2"][1], totals["S1"][0],
                   totals["S1"][1], totals["S3"][0], totals["S3"][1])),
            req("Chapter 5 complete", False, "5.4 is blocked"),
        ],
    })

    for entry in (
        ("ch4-429", "chapters/04-system.tex:429",
         "Half page in your own words", "approx. half a page"),
        ("ch4-632", "chapters/04-system.tex:632",
         "Two screenshots of the assistant", ""),
        ("zusammenfassung", "frontmatter/zusammenfassung.tex",
         "Zusammenfassung, German translation of the abstract", ""),
        ("declaration", "frontmatter/declaration.tex",
         "Declaration of Authorship", ""),
        ("title-date", "main.tex:104",
         "Submission date on the title page", ""),
    ):
        items.append({
            "id": entry[0], "where": entry[1], "title": entry[2],
            "pages": entry[3], "action": "Not blocked by measurement.",
            "requirements": [],
        })

    for item in items:
        reqs = item["requirements"]
        if not reqs:
            item["status"] = "NOT DATA DEPENDENT"
        elif all(r["ok"] for r in reqs):
            item["status"] = "COMPLETE"
        elif any(r["ok"] for r in reqs):
            item["status"] = "PARTIAL"
        else:
            item["status"] = "BLOCKED"
    return items


def e(text):
    return html.escape(str(text), quote=True)


def fmt(value, spec="%.4f"):
    if value is None:
        return "&mdash;"
    return spec % value


CSS = """
body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
       margin: 0; padding: 2rem 3rem 5rem; color: #16181d; background: #fbfbfc;
       line-height: 1.5; }
h1 { font-size: 1.6rem; margin: 0 0 .25rem; }
h2 { font-size: 1.15rem; margin: 2.5rem 0 .75rem; padding-bottom: .3rem;
     border-bottom: 2px solid #16181d; }
h3 { font-size: .95rem; margin: 1.5rem 0 .5rem; }
.sub { color: #626772; font-size: .85rem; margin-bottom: .35rem; }
table { border-collapse: collapse; width: 100%; font-size: .82rem;
        background: #fff; margin-bottom: .5rem; }
th, td { text-align: left; padding: .4rem .55rem; border-bottom: 1px solid #e6e8ec; }
th { background: #f2f3f5; font-weight: 600; white-space: nowrap; }
td.num, th.num { text-align: right; font-variant-numeric: tabular-nums; }
tr.groupend td { border-bottom: 2px solid #c8ccd4; }
.tag { display: inline-block; padding: .1rem .45rem; border-radius: 3px;
       font-size: .7rem; font-weight: 700; letter-spacing: .02em; }
.COMPLETE { background: #d8f0dc; color: #14532d; }
.PARTIAL { background: #fdeecd; color: #7c4a03; }
.BLOCKED { background: #fbdada; color: #7f1d1d; }
.NOTDATA { background: #e6e8ec; color: #40454f; }
.item { background: #fff; border: 1px solid #e6e8ec; border-left: 4px solid #c8ccd4;
        padding: .7rem .9rem; margin-bottom: .6rem; }
.item.BLOCKED { border-left-color: #dc2626; }
.item.PARTIAL { border-left-color: #f59e0b; }
.item.COMPLETE { border-left-color: #16a34a; }
.item h4 { margin: 0 0 .1rem; font-size: .92rem; }
.where { font-family: ui-monospace, SFMono-Regular, Menlo, monospace;
         font-size: .74rem; color: #626772; }
ul.req { margin: .5rem 0 0; padding-left: 1.1rem; font-size: .8rem; }
ul.req li { margin-bottom: .15rem; }
li.no { color: #7f1d1d; }
li.yes { color: #40454f; }
.detail { color: #626772; }
.note { background: #fff8e6; border-left: 4px solid #f59e0b; padding: .6rem .9rem;
        font-size: .84rem; margin: .6rem 0; }
.sig { font-weight: 700; color: #14532d; }
.nsig { color: #7f1d1d; }
footer { margin-top: 3rem; font-size: .75rem; color: #626772;
         border-top: 1px solid #e6e8ec; padding-top: .8rem; }
"""


def render(campaign_dir, records, meta):
    projects, loc, grid = build_cells(records)
    totals = accuracy_totals(grid, projects)
    acc = accuracy_tests(totals)
    cost_tests = paired_tests(grid, projects, "cost_values")
    prompt_tests = paired_tests(grid, projects, "prompt_values")
    meta = dict(meta)
    meta["_dir"] = campaign_dir
    items = evidence_map(records, grid, projects, loc, meta, totals, acc)
    pricing = meta.get("pricing") or {}
    total_cost = sum(r.get("cost_usd") or 0.0 for r in records)

    out = []
    a = out.append
    a("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>")
    a("<title>Thesis evidence map</title><style>%s</style></head><body>" % CSS)
    a("<h1>Thesis evidence map</h1>")
    a("<div class='sub'>%s &middot; %d measured runs &middot; %d projects &middot; "
      "%s &middot; %.4f USD total &middot; generated %s</div>"
      % (e(os.path.basename(campaign_dir)), len(records), len(projects),
         e(pricing.get("model", "unknown model")), total_cost,
         datetime.datetime.now().strftime("%Y-%m-%d %H:%M")))

    a("<div class='note'><strong>These charts are not thesis figures.</strong> "
      "The supervisor's rules require vector PDF drawn by you. The pgfplots "
      "sources for the chapter are written to <code>figures/</code> by this same "
      "script, from this same data, so the two cannot disagree.</div>")

    counts = {}
    for item in items:
        counts[item["status"]] = counts.get(item["status"], 0) + 1
    a("<h2>Remaining thesis items</h2>")
    a("<div class='sub'>%s</div>" % " &middot; ".join(
        "%d %s" % (counts[k], k.lower()) for k in
        sorted(counts, key=lambda s: STATUS_ORDER.get(s, 9))))

    for item in sorted(items, key=lambda i: STATUS_ORDER.get(i["status"], 9)):
        cls = item["status"].replace(" ", "")
        tag = "NOTDATA" if item["status"] == "NOT DATA DEPENDENT" else item["status"]
        a("<div class='item %s'>" % e(item["status"].split()[0]))
        a("<h4>%s <span class='tag %s'>%s</span></h4>"
          % (e(item["title"]), tag, e(item["status"])))
        a("<div class='where'>%s%s</div>"
          % (e(item["where"]), (" &middot; " + e(item["pages"])) if item["pages"] else ""))
        if item["requirements"]:
            a("<ul class='req'>")
            for r in item["requirements"]:
                mark = "yes" if r["ok"] else "no"
                sym = "&#10003;" if r["ok"] else "&#10007;"
                a("<li class='%s'>%s %s%s</li>"
                  % (mark, sym, e(r["label"]),
                     (" &mdash; <span class='detail'>%s</span>" % e(r["detail"]))
                     if r["detail"] else ""))
            a("</ul>")
        a("<div class='sub' style='margin-top:.4rem'>%s</div>" % e(item["action"]))
        a("</div>")

    a("<h2>Results by condition</h2>")
    a("<table><tr><th>project</th><th class='num'>LOC</th><th class='num'>files</th>"
      "<th>strategy</th><th class='num'>selected</th><th class='num'>success</th>"
      "<th class='num'>prompt</th><th class='num'>output</th>"
      "<th class='num'>thinking</th><th class='num'>cost USD</th>"
      "<th class='num'>95% CI</th></tr>")
    for project in projects:
        for i, strategy in enumerate(STRATEGIES):
            c = grid.get((project, strategy))
            if not c:
                continue
            last = " class='groupend'" if i == len(STRATEGIES) - 1 else ""
            a("<tr%s><td>%s</td><td class='num'>%d</td><td class='num'>%d</td>"
              "<td>%s <span class='detail'>%s</span></td><td class='num'>%s</td>"
              "<td class='num'>%d/%d</td><td class='num'>%.0f</td>"
              "<td class='num'>%.0f</td><td class='num'>%.0f &plusmn; %.0f</td>"
              "<td class='num'>%.5f</td><td class='num detail'>%s&ndash;%s</td></tr>"
              % (last, e(project), c["loc"], c["files"], strategy,
                 e(STRATEGY_LABEL[strategy]), c["selected"], c["ok"], c["n"],
                 c["prompt"]["mean"] or 0, c["output"]["mean"] or 0,
                 c["think"]["mean"] or 0, c["think"]["sd"] or 0,
                 c["cost"]["mean"] or 0,
                 fmt(c["cost"]["lo"], "%.5f"), fmt(c["cost"]["hi"], "%.5f")))
    a("</table>")

    a("<h2>Does accuracy differ between strategies?</h2>")
    a("<div class='sub'>Fisher exact test on the pooled two-by-two table, "
      "all projects together.</div>")
    a("<table><tr><th>comparison</th><th class='num'>first</th>"
      "<th class='num'>second</th><th class='num'>p</th><th>verdict</th></tr>")
    for row in acc:
        a("<tr><td>%s</td><td class='num'>%s</td><td class='num'>%s</td>"
          "<td class='num'>%.4f</td><td class='%s'>%s</td></tr>"
          % (e(row["pair"]), e(row["a"]), e(row["b"]), row["p"],
             "sig" if row["significant"] else "nsig",
             "significant at 0.05" if row["significant"]
             else "not significant"))
    a("</table>")

    for title, rows, unit in (
        ("Does cost differ between strategies?", cost_tests, "%.5f"),
        ("Do prompt tokens differ between strategies?", prompt_tests, "%.0f"),
    ):
        a("<h2>%s</h2>" % e(title))
        a("<div class='sub'>Mann-Whitney exact test with Cliff's delta as the "
          "effect size, computed per project.</div>")
        a("<table><tr><th>project</th><th>comparison</th><th class='num'>first</th>"
          "<th class='num'>second</th><th class='num'>p</th>"
          "<th class='num'>delta</th><th>effect</th></tr>")
        for row in rows:
            a("<tr><td>%s</td><td>%s</td><td class='num'>%s</td>"
              "<td class='num'>%s</td><td class='num %s'>%.4f</td>"
              "<td class='num'>%+.2f</td><td>%s</td></tr>"
              % (e(row["project"]), e(row["pair"]), unit % row["mean_a"],
                 unit % row["mean_b"], "sig" if row["significant"] else "nsig",
                 row["p"], row["delta"], e(row["magnitude"])))
        a("</table>")

    a("<h2>Failure modes</h2>")
    a("<table><tr><th>strategy</th><th>kind</th><th class='num'>count</th></tr>")
    for (strategy, kind), count in failure_modes(records):
        a("<tr><td>%s</td><td>%s</td><td class='num'>%d</td></tr>"
          % (e(strategy), e(kind), count))
    a("</table>")

    a("<footer>Generated from <code>%s/results.jsonl</code>. "
      "Regenerate with <code>python -m harness.dashboard %s</code> rather than "
      "editing this file.</footer>" % (e(campaign_dir), e(campaign_dir)))
    a("</body></html>")
    return "\n".join(out)


AXIS = (
    "    width=12cm, height=6.2cm,\n"
    "    font=\\small,\n"
    "    legend pos=north east,\n"
    "    legend cell align=left,\n"
    "    legend style={draw=black!30, fill=white, font=\\small},\n"
    "    ymajorgrids=true,\n"
    "    grid style={black!15},\n"
    "    tick label style={font=\\small},\n"
    "    label style={font=\\small},\n"
    "    enlarge y limits={upper, value=0.32},\n"
)

MARKS = {"S1": "*", "S2": "square*", "S3": "triangle*"}
DASH = {"S1": "solid", "S2": "densely dashed", "S3": "densely dotted"}


def _figure(label, body, macro):
    return (
        "\\begin{figure}[htbp]\n\\centering\n"
        + body
        + "\\caption{\\" + macro + "}\n"
        + "\\label{" + label + "}\n\\end{figure}\n"
    )


CAPTION_MACROS = (
    ("CaptionAccuracy", "what this shows about RQ1, in your own words"),
    ("CaptionPromptScaling",
     "how the prompt grows with project size under each strategy"),
    ("CaptionCost", "what this shows about RQ2, in your own words"),
    ("CaptionRatio", "what the trend means for RQ3, in your own words"),
    ("CaptionAccuracyTable", "the per-condition success rates"),
    ("CaptionCostTable", "the per-condition token counts and cost"),
    ("CaptionStatsTable", "the inference statistics"),
)


def write_captions(target):
    path = os.path.join(target, "captions.tex")
    if os.path.exists(path):
        return path, False
    lines = []
    for macro, todo in CAPTION_MACROS:
        lines.append("\\newcommand{\\%s}{\\textcolor{red}{TODO (Umair): %s}}\n"
                     % (macro, todo))
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("".join(lines))
    return path, True


def figure_accuracy(projects, grid):
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", AXIS,
            "    ybar, bar width=7pt,\n",
            "    ylabel={Fully successful runs (per cent)},\n",
            "    xlabel={Project, ordered by size},\n",
            "    symbolic x coords={" + ",".join(projects) + "},\n",
            "    xtick=data, ymin=0, ytick={0,25,50,75,100},\n",
            "    enlarge x limits=0.18,\n]\n"]
    for strategy in STRATEGIES:
        pts = []
        for project in projects:
            c = grid.get((project, strategy))
            if c:
                pts.append("(%s,%.1f)" % (project, 100.0 * c["ok"] / c["n"]))
        body.append("\\addplot coordinates {" + " ".join(pts) + "};\n")
        body.append("\\addlegendentry{%s %s}\n"
                    % (strategy, STRATEGY_LABEL[strategy]))
    body.append("\\end{axis}\n\\end{tikzpicture}\n")
    return _figure("fig:accuracy", "".join(body), "CaptionAccuracy")


def figure_scaling(projects, loc, grid, key, ylabel, label, todo, scale=1.0):
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", AXIS,
            "    ylabel={" + ylabel + "},\n",
            "    xlabel={Project size (lines of code)},\n",
            "    enlarge y limits={upper, value=0.55},\n",
            "    xmin=0, ymin=0,\n]\n"]
    for strategy in STRATEGIES:
        pts = []
        for project in projects:
            c = grid.get((project, strategy))
            if c and c[key]["mean"] is not None:
                pts.append("(%d,%.5f)" % (loc[project], c[key]["mean"] * scale))
        body.append("\\addplot[mark=%s, %s] coordinates {%s};\n"
                    % (MARKS[strategy], DASH[strategy], " ".join(pts)))
        body.append("\\addlegendentry{%s %s}\n"
                    % (strategy, STRATEGY_LABEL[strategy]))
    body.append("\\end{axis}\n\\end{tikzpicture}\n")
    return _figure(label, "".join(body), todo)


def figure_ratio(projects, loc, grid):
    pts = []
    for project in projects:
        s1, s3 = grid.get((project, "S1")), grid.get((project, "S3"))
        if s1 and s3 and s1["prompt"]["mean"]:
            pts.append("(%d,%.1f)"
                       % (loc[project],
                          100.0 * s3["prompt"]["mean"] / s1["prompt"]["mean"]))
    body = ["\\begin{tikzpicture}\n\\begin{axis}[\n", AXIS,
            "    ylabel={S3 prompt as a percentage of S1},\n",
            "    xlabel={Project size (lines of code)},\n",
            "    xmin=0, ymin=0, ytick={0,25,50,75,100},\n]\n",
            "\\addplot[mark=triangle*, densely dotted] coordinates {"
            + " ".join(pts) + "};\n",
            "\\addlegendentry{S3 relative to S1}\n",
            "\\end{axis}\n\\end{tikzpicture}\n"]
    return _figure("fig:ratio", "".join(body), "CaptionRatio")


def _table(label, macro, spec, header, rows):
    out = ["\\begin{table}[htbp]\n\\centering\n\\small\n",
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


def table_accuracy(projects, loc, grid):
    rows = []
    for i, project in enumerate(projects):
        if i:
            rows.append(None)
        first = True
        for strategy in STRATEGIES:
            c = grid.get((project, strategy))
            if not c:
                continue
            rows.append("%s & %s & %s & %s & %d & %d/%d & %.0f"
                        % (project if first else "",
                           str(loc[project]) if first else "",
                           str(c["files"]) if first else "",
                           strategy, c["selected"] or 0, c["ok"], c["n"],
                           100.0 * c["ok"] / c["n"]))
            first = False
    return _table(
        "tab:accuracy", "CaptionAccuracyTable", "lrrlrrr",
        "Project & LOC & Files & Str. & Sel. & Successful & Per cent",
        rows)


def table_cost(projects, grid):
    rows = []
    for i, project in enumerate(projects):
        if i:
            rows.append(None)
        first = True
        for strategy in STRATEGIES:
            c = grid.get((project, strategy))
            if not c:
                continue
            rows.append("%s & %s & %.0f & %.0f & %.0f (%.0f) & %.4f (%.4f)"
                        % (project if first else "", strategy,
                           c["prompt"]["mean"] or 0.0,
                           c["output"]["mean"] or 0.0,
                           c["think"]["mean"] or 0.0,
                           c["think"]["sd"] or 0.0,
                           c["cost"]["mean"] or 0.0,
                           c["cost"]["sd"] or 0.0))
            first = False
    return _table(
        "tab:cost", "CaptionCostTable", "llrrrr",
        "Project & Str. & Prompt & Output & Thinking (sd) & Cost USD (sd)",
        rows)


def table_stats(acc_tests, cost_tests, prompt_tests):
    rows = []
    for t in acc_tests:
        rows.append("Accuracy, pooled & %s & %s vs %s & %s & %s"
                    % (t["pair"], t["a"], t["b"], fmt(t["p"], "%.4f"),
                       "yes" if t["significant"] else "no"))
    rows.append(None)
    for t in prompt_tests:
        if t["pair"] != "S1 vs S3":
            continue
        rows.append("Prompt tokens & %s & %s & %s & %s"
                    % (t["project"], t["pair"], fmt(t["p"], "%.4f"),
                       "yes" if t["significant"] else "no"))
    rows.append(None)
    for t in cost_tests:
        if t["pair"] != "S1 vs S3":
            continue
        rows.append("Cost & %s & %s & %s & %s"
                    % (t["project"], t["pair"], fmt(t["p"], "%.4f"),
                       "yes" if t["significant"] else "no"))
    return _table(
        "tab:stats", "CaptionStatsTable", "lllrl",
        "Quantity & Scope & Comparison & $p$ & Significant",
        rows)


def write_figures(target, projects, loc, grid, totals=None, acc_tests=None):
    if not os.path.isdir(target):
        os.makedirs(target)
    files = {
        "ch5-accuracy.tex": figure_accuracy(projects, grid),
        "ch5-prompt-scaling.tex": figure_scaling(
            projects, loc, grid, "prompt", "Mean prompt tokens",
            "fig:promptscaling", "CaptionPromptScaling"),
        "ch5-cost.tex": figure_scaling(
            projects, loc, grid, "cost",
            "Mean cost per run (USD cents)", "fig:cost",
            "CaptionCost", scale=100.0),
        "ch5-ratio.tex": figure_ratio(projects, loc, grid),
        "ch5-tab-accuracy.tex": table_accuracy(projects, loc, grid),
        "ch5-tab-cost.tex": table_cost(projects, grid),
    }
    if acc_tests is not None:
        files["ch5-tab-stats.tex"] = table_stats(
            acc_tests,
            paired_tests(grid, projects, "cost_values"),
            paired_tests(grid, projects, "prompt_values"))
    written = []
    for name, content in files.items():
        path = os.path.join(target, name)
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(content)
        written.append(path)
    caption_path, created = write_captions(target)
    written.append(caption_path + ("" if created else "  (kept, not overwritten)"))
    return written


def main(argv=None):
    argv = argv if argv is not None else sys.argv[1:]
    if not argv:
        print("usage: python -m harness.dashboard <campaign_dir> [--figures <dir>]")
        return 2
    campaign_dir = argv[0]
    target = "figures"
    if "--figures" in argv:
        target = argv[argv.index("--figures") + 1]

    records = load(campaign_dir)
    if not records:
        print("no measured runs in %s" % campaign_dir)
        return 1
    meta = campaign_meta(campaign_dir)

    page = render(campaign_dir, records, meta)
    out_path = os.path.join(campaign_dir, "dashboard.html")
    with open(out_path, "w", encoding="utf-8") as fh:
        fh.write(page)

    projects, loc, grid = build_cells(records)
    totals = accuracy_totals(grid, projects)
    written = write_figures(target, projects, loc, grid, totals,
                            accuracy_tests(totals))

    print("dashboard  %s" % out_path)
    for path in written:
        print("figure     %s" % path)
    print("\nadd to the preamble if it is not there yet:")
    print("  \\usepackage{pgfplots}")
    print("  \\pgfplotsset{compat=1.18}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
