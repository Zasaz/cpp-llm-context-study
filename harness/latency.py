import os
import statistics
import sys

from harness import blocked
from harness.report import load

STRATEGIES = ("S1", "S2", "S3")
FIELDS = (("model_latency_s", "model"),
          ("execution_latency_s", "execution"),
          ("request_latency_s", "request"))


def pick(runs, field):
    if field == "model_latency_s":
        values = [(r.get("model_call") or {}).get(field) for r in runs]
    else:
        values = [r.get(field) for r in runs]
    return [v for v in values if isinstance(v, (int, float))]


def cell_text(runs, field):
    values = pick(runs, field)
    if not values:
        return "-"
    sd = statistics.stdev(values) if len(values) > 1 else 0.0
    return "%.1f (%.1f)" % (statistics.mean(values), sd)


def render(arms):
    out = []
    w = out.append
    w("# Latency")
    w("")
    w("Per-run wall-clock latencies in seconds, mean (sample sd) over the ten")
    w("repetitions of each cell. The model latency is the time the API call")
    w("took, the execution latency is the compile-and-run step of the grader,")
    w("and the request latency is the full round trip through the service")
    w("including context assembly.")
    w("")
    for key, payload in arms.items():
        records = payload["records"]
        projects, loc, grid = blocked.cells(records)
        w("## %s arm (`%s`)" % (key, payload["dir"]))
        w("")
        for field, label in FIELDS:
            w("### %s latency" % label)
            w("")
            w("| project | %s |" % " | ".join(STRATEGIES))
            w("|---|---|---|---|")
            for project in projects:
                cells_row = [cell_text(grid[(project, s)], field)
                             for s in STRATEGIES]
                w("| %s | %s |" % (project, " | ".join(cells_row)))
            pooled = ["%.1f" % statistics.mean(
                [v for p in projects for v in pick(grid[(p, s)], field)])
                for s in STRATEGIES]
            w("| all, mean | %s |" % " | ".join(pooled))
            w("")
        total = sum(v for v in (r.get("request_latency_s") for r in records)
                    if isinstance(v, (int, float)))
        w("Total request wall clock across the %d runs: %.0f s." % (
            len(records), total))
        w("")
    return "\n".join(out) + "\n"


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    paths = {}
    out_path = None
    i = 0
    while i < len(argv):
        if argv[i] == "--default":
            paths["default"] = argv[i + 1]
            i += 2
        elif argv[i] == "--suppressed":
            paths["suppressed"] = argv[i + 1]
            i += 2
        elif argv[i] == "--out":
            out_path = argv[i + 1]
            i += 2
        else:
            print("usage: python -m harness.latency --default DIR "
                  "--suppressed DIR [--out FILE]")
            return 2
    if "default" not in paths or "suppressed" not in paths:
        print("usage: python -m harness.latency --default DIR "
              "--suppressed DIR [--out FILE]")
        return 2
    arms = {}
    for key in ("default", "suppressed"):
        records = load(paths[key])
        if not records:
            raise SystemExit("no measured runs in %s" % paths[key])
        arms[key] = {"dir": paths[key], "records": records}
    text = render(arms)
    if out_path:
        with open(out_path, "w", encoding="utf-8") as fh:
            fh.write(text)
        print("wrote %s" % out_path)
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
