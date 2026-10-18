import json
import os
import statistics
import sys


def load(campaign_dir):
    records = []
    path = os.path.join(campaign_dir, "results.jsonl")
    with open(path, "r", encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            record = json.loads(line)
            if not record.get("dry_run"):
                records.append(record)
    return records


def spread(values):
    values = [v for v in values if isinstance(v, (int, float))]
    if not values:
        return None, None
    if len(values) == 1:
        return values[0], 0.0
    return statistics.mean(values), statistics.stdev(values)


def thought_tokens(call):
    call = call or {}
    recorded = call.get("thought_tokens")
    if isinstance(recorded, int):
        return recorded
    total = call.get("total_tokens")
    if not isinstance(total, int):
        return None
    residual = total - (call.get("prompt_tokens") or 0) - (call.get("output_tokens") or 0)
    return residual if residual >= 0 else None


def by_condition(records):
    buckets = {}
    for record in records:
        buckets.setdefault((record["project"], record["strategy"]), []).append(record)
    return buckets


def report(campaign_dir):
    records = load(campaign_dir)
    if not records:
        print("no measured runs in %s" % campaign_dir)
        return 1

    buckets = by_condition(records)
    order = {}
    for record in records:
        order[record["project"]] = record["project_metrics"]["loc"]
    projects = sorted(order, key=lambda p: order[p])

    print("campaign %s" % campaign_dir)
    print("%d measured runs, %d projects\n" % (len(records), len(projects)))

    print("%-11s %-4s %5s %5s %6s %7s %7s %13s %9s" % (
        "project", "str", "loc", "files", "sel", "ok", "prompt", "think (sd)",
        "cost"))
    print("-" * 78)

    for project in projects:
        for strategy in ("S1", "S2", "S3"):
            runs = buckets.get((project, strategy))
            if not runs:
                continue
            grades = [r.get("grade") or {} for r in runs]
            calls = [r.get("model_call") or {} for r in runs]
            metrics = runs[0]["project_metrics"]
            selected = (runs[0].get("context_meta") or {}).get("files_selected")
            ok = sum(1 for g in grades if g.get("fully_successful"))
            prompt_mean, _ = spread([c.get("prompt_tokens") for c in calls])
            think_mean, think_sd = spread([thought_tokens(c) for c in calls])
            cost_mean, cost_sd = spread([r.get("cost_usd") for r in runs])
            print("%-11s %-4s %5d %5d %5s %5s %7.0f %7.0f (%4.0f) %9.5f" % (
                project, strategy, metrics["loc"], metrics["files"],
                selected, "%d/%d" % (ok, len(runs)),
                prompt_mean or 0, think_mean or 0, think_sd or 0,
                cost_mean or 0))
        print()

    print("failure modes")
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
        kinds.setdefault((record["strategy"], kind), 0)
        kinds[(record["strategy"], kind)] += 1
    if kinds:
        for (strategy, kind), count in sorted(kinds.items()):
            print("  %-4s %-18s %d" % (strategy, kind, count))
    else:
        print("  none")

    print("\nwhether cost separates the strategies")
    for project in projects:
        gaps = []
        widest_sd = 0.0
        for strategy in ("S1", "S2", "S3"):
            runs = buckets.get((project, strategy))
            if not runs:
                continue
            mean, sd = spread([r.get("cost_usd") for r in runs])
            if mean is None:
                continue
            gaps.append(mean)
            widest_sd = max(widest_sd, sd or 0.0)
        if len(gaps) < 2:
            continue
        span = max(gaps) - min(gaps)
        verdict = "not distinguishable" if span < widest_sd else "possibly separable"
        print("  %-11s spread between strategies %.5f, widest within-condition sd "
              "%.5f, %s" % (project, span, widest_sd, verdict))

    thinking = [thought_tokens(r.get("model_call")) for r in records]
    thinking = [t for t in thinking if isinstance(t, int)]
    if thinking:
        silent = sum(1 for t in thinking if t == 0)
        print("\nthinking tokens over the whole campaign: min %d, max %d, mean %.0f, "
              "sd %.0f, %d of %d runs did no thinking"
              % (min(thinking), max(thinking), statistics.mean(thinking),
                 statistics.stdev(thinking) if len(thinking) > 1 else 0.0,
                 silent, len(thinking)))
    return 0


def main(argv=None):
    argv = argv if argv is not None else sys.argv[1:]
    if not argv:
        print("usage: python -m harness.report <campaign_dir>")
        return 2
    return report(argv[0])


if __name__ == "__main__":
    sys.exit(main())
