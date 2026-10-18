import json
import os
import statistics
import sys

sys.path.insert(0, "/Users/umair1/Work/Project/cpp_helper_bot-master")
os.chdir("/Users/umair1/Work/Project/cpp_helper_bot-master")

from harness import dashboard

campaign = sys.argv[1]
records = [json.loads(l) for l in
           open(os.path.join(campaign, "results.jsonl")) if l.strip()]
meta = dashboard.campaign_meta(campaign)
manifest_path = os.path.join(campaign, "manifest.json")
if os.path.exists(manifest_path):
    with open(manifest_path) as fh:
        merged = json.load(fh)
    merged.update(meta)
    meta = merged
projects, loc, grid = dashboard.build_cells(records)
totals = dashboard.accuracy_totals(grid, projects)
acc = dashboard.accuracy_tests(totals)

out = []
w = out.append


def n(value, spec="%.0f"):
    return "-" if value is None else spec % value


w("# Chapter 5 numbers sheet")
w("")
w("Generated from `%s` by `harness.dashboard`, not typed by hand. Every figure"
  % campaign)
w("Chapter 5 would cite is here, so an independent calculation can be checked")
w("against it without reading any prose.")
w("")
w("## Campaign identity")
w("")
env = meta.get("environment") or {}
env_note = ""
if not env:
    post = os.path.join(campaign, "environment-postrecorded.json")
    if os.path.exists(post):
        with open(post) as fh:
            env = json.load(fh)
        env_note = (" (recorded after the campaign on the same machine and "
                    "virtual environment, not contemporaneously)")
for k, v in (("started", meta.get("started")),
             ("model requested", meta.get("model_requested")),
             ("harness version", meta.get("harness_version")),
             ("strategy version", meta.get("strategy_version")),
             ("extractor version", meta.get("extractor_version")),
             ("repetitions per cell", meta.get("repetitions")),
             ("thinking", meta.get("thinking", "not recorded, campaign predates the flag")),
             ("generation params", meta.get("generation_params")),
             ("uncontrolled params", meta.get("uncontrolled_params")),
             ("pricing", meta.get("pricing")),
             ("python", env.get("python")),
             ("platform", env.get("platform")),
             ("google-genai", env.get("google_genai")),
             ("compiler", env.get("compiler")),
             ("compile flags", env.get("compile_flags"))):
    w("- %s: `%s`" % (k, v))
w("- measured runs: `%d`" % len(records))
w("- total cost: `$%.4f`" % sum(r.get("cost_usd") or 0.0 for r in records))
if env_note:
    w("- environment%s" % env_note)
unc = str(meta.get("uncontrolled_params") or "")
if "not settable" in unc:
    w("")
    w("The `uncontrolled_params` string above is wrong and is left as recorded,")
    w("because rewriting a frozen manifest would falsify the record. Thinking is")
    w("settable through `google-genai` 1.47.0 by way of")
    w("`types.ThinkingConfig(thinking_budget=0)`, verified 2026-08-21. This")
    w("campaign simply did not set it.")
w("")

w("## Corpus")
w("")
w("| project | LOC | files | symbols | classes | functions | call edges | active file |")
w("|---|---|---|---|---|---|---|---|")
seen = {}
for r in records:
    if r["project"] not in seen:
        seen[r["project"]] = (r.get("project_metrics") or {}, r.get("active_file"))
for p in projects:
    m, af = seen.get(p, ({}, None))
    w("| %s | %s | %s | %s | %s | %s | %s | `%s` |" % (
        p, m.get("loc"), m.get("files"), m.get("symbols"), m.get("classes"),
        m.get("functions"), m.get("call_edges"), af))
w("")

w("## Per condition")
w("")
w("Mean over the repetitions, with the standard deviation in brackets.")
w("")
w("| project | str | sel | ok | prompt tok | output tok | think tok | cost USD |")
w("|---|---|---|---|---|---|---|---|")
for p in projects:
    for s in ("S1", "S2", "S3"):
        c = grid.get((p, s))
        if not c:
            continue
        def pair(d, spec="%.0f"):
            return "%s (%s)" % (n(d.get("mean"), spec), n(d.get("sd"), spec))
        w("| %s | %s | %s | %d/%d | %s | %s | %s | %s |" % (
            p, s, c.get("selected"), c["ok"], c["n"],
            pair(c["prompt"]), pair(c["output"]), pair(c["think"]),
            pair(c["cost"], "%.5f")))
w("")

w("### Cost with a 95 per cent bootstrap interval")
w("")
w("| project | str | mean cost USD | lo | hi |")
w("|---|---|---|---|---|")
for p in projects:
    for s in ("S1", "S2", "S3"):
        c = grid.get((p, s))
        if not c:
            continue
        d = c["cost"]
        w("| %s | %s | %s | %s | %s |" % (
            p, s, n(d.get("mean"), "%.5f"), n(d.get("lo"), "%.5f"),
            n(d.get("hi"), "%.5f")))
w("")

w("## Pooled accuracy")
w("")
w("| strategy | successful | runs | per cent |")
w("|---|---|---|---|")
for s in ("S1", "S2", "S3"):
    ok, total = totals[s]
    w("| %s | %d | %d | %.1f |" % (s, ok, total, 100.0 * ok / total))
w("")

w("## Inference statistics")
w("")
w("### Accuracy, Fisher exact on the pooled two-by-two")
w("")
w("| comparison | a | b | p | significant at 0.05 |")
w("|---|---|---|---|---|")
for t in acc:
    w("| %s | %s | %s | %.4f | %s |" % (
        t["pair"], t["a"], t["b"], t["p"], "yes" if t["significant"] else "no"))
w("")

for key, label in (("prompt_values", "Prompt tokens"),
                   ("cost_values", "Cost USD")):
    w("### %s, Mann-Whitney with Cliff's delta, per project" % label)
    w("")
    w("| project | comparison | mean a | mean b | p | delta | magnitude | significant |")
    w("|---|---|---|---|---|---|---|---|")
    for t in dashboard.paired_tests(grid, projects, key):
        spec = "%.5f" if key == "cost_values" else "%.0f"
        w("| %s | %s | %s | %s | %s | %+.2f | %s | %s |" % (
            t["project"], t["pair"], spec % t["mean_a"], spec % t["mean_b"],
            n(t["p"], "%.4f"), t["delta"], t["magnitude"],
            "yes" if t["significant"] else "no"))
    w("")

w("## Failure modes")
w("")
w("| strategy | mode | count |")
w("|---|---|---|")
for (strategy, kind), count in dashboard.failure_modes(records):
    w("| %s | %s | %d |" % (strategy, kind, count))
w("")

w("## Degenerate conditions")
w("")
deg = dashboard.degenerate_conditions(grid, projects)
w("Projects where S3 selected the same number of files as S1, so the two")
w("conditions are not distinct:")
w("")
if deg:
    for d in deg:
        w("- %s" % d)
else:
    w("- none")
w("")

w("## Thinking tokens over the whole campaign")
w("")
th = [dashboard.thought_tokens(r.get("model_call") or {}) for r in records]
th = [t for t in th if isinstance(t, int)]
if th:
    w("- recovered for `%d of %d` runs" % (len(th), len(records)))
    w("- min `%d`, max `%d`, mean `%.0f`, sd `%.0f`" % (
        min(th), max(th), statistics.mean(th),
        statistics.stdev(th) if len(th) > 1 else 0.0))
    w("- runs that did no thinking: `%d`" % sum(1 for t in th if t == 0))
w("")

w("## Cost decomposition")
w("")
w("What share of the bill each token class carries, pooled over projects.")
w("")
pr = meta.get("pricing") or {}
rate_in = (pr.get("input_per_million_usd") or 0.75) / 1e6
rate_out = (pr.get("output_per_million_usd") or 3.75) / 1e6
w("| strategy | prompt | output | thinking | prompt share | thinking share |")
w("|---|---|---|---|---|---|")
share = {}
for s in ("S1", "S2", "S3"):
    ps, os_, ts = [], [], []
    for p in projects:
        c = grid.get((p, s))
        if not c:
            continue
        for m, acc_list in ((c["prompt"], ps), (c["output"], os_), (c["think"], ts)):
            if m.get("mean") is not None:
                acc_list.append(m["mean"])
    mp, mo, mt = statistics.mean(ps), statistics.mean(os_), statistics.mean(ts)
    total = mp * rate_in + (mo + mt) * rate_out
    share[s] = (mp, mo, mt, total)
    w("| %s | %.0f | %.0f | %.0f | %.1f%% | %.1f%% |" % (
        s, mp, mo, mt, 100 * mp * rate_in / total, 100 * mt * rate_out / total))
w("")
w("S3 against S1, as measured and with the thinking component removed. The")
w("second column is an estimate, not a measurement, because output tokens also")
w("move when thinking is suppressed.")
w("")
w("| ratio | as measured | thinking removed |")
w("|---|---|---|")
a, b = share["S1"], share["S3"]
w("| S3/S1 cost | %.3f | %.3f |" % (
    b[3] / a[3],
    (b[0] * rate_in + b[1] * rate_out) / (a[0] * rate_in + a[1] * rate_out)))
w("| S3/S1 prompt tokens | %.3f | same |" % (b[0] / a[0]))
w("")

sys.stdout.write("\n".join(out) + "\n")
