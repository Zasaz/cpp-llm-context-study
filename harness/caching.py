import os
import statistics
import sys

from harness import stats
from harness.report import load, thought_tokens

ARM_KEYS = ("default", "suppressed")

ARM_LABEL = {"default": "Default", "suppressed": "Suppressed"}

INPUT_PER_TOKEN = 0.75e-6
OUTPUT_PER_TOKEN = 3.75e-6
CACHED_INPUT_PER_TOKEN = 0.075e-6

STRATEGIES = ("S1", "S2", "S3")


def _costs(call):
    prompt = call.get("prompt_tokens") or 0
    output = call.get("output_tokens") or 0
    thought = thought_tokens(call) or 0
    cached = call.get("cached_tokens") or 0
    billed_output = (output + thought) * OUTPUT_PER_TOKEN
    uncached = prompt * INPUT_PER_TOKEN + billed_output
    adjusted = ((prompt - cached) * INPUT_PER_TOKEN
                + cached * CACHED_INPUT_PER_TOKEN
                + billed_output)
    return uncached, adjusted, cached


def build(paths):
    arms = {}
    for key, path in paths.items():
        cells = {}
        for record in load(path):
            call = record.get("model_call") or {}
            uncached, adjusted, cached = _costs(call)
            cell = cells.setdefault((record["project"], record["strategy"]),
                                    {"uncached": [], "adjusted": [],
                                     "cached": [], "overhead": [],
                                     "loc": record["project_metrics"]["loc"]})
            cell["uncached"].append(uncached)
            cell["adjusted"].append(adjusted)
            cell["cached"].append(cached)
            cell["overhead"].append(record.get("overhead_latency_s") or 0.0)
        arms[key] = cells
    return arms


def _saving(cells, project, field):
    base = cells[(project, "S1")][field]
    variant = cells[(project, "S3")][field]
    a = statistics.mean(base)
    b = statistics.mean(variant)
    return (a - b) / a * 100.0 if a else None


def cached_cells(arms):
    hits = []
    for key in ARM_KEYS:
        for (project, strategy), cell in sorted(arms[key].items()):
            n = sum(1 for c in cell["cached"] if c)
            if n:
                tokens = sorted({c for c in cell["cached"] if c})
                hits.append((key, project, strategy, n, len(cell["cached"]),
                             tokens))
    return hits


def crossing(cells, field, low, high):
    a = _saving(cells, low, field)
    b = _saving(cells, high, field)
    if a is None or b is None or (a < 0) == (b < 0):
        return None
    loc_low = cells[(low, "S1")]["loc"]
    loc_high = cells[(high, "S1")]["loc"]
    return loc_low + (loc_high - loc_low) * abs(a) / (abs(a) + abs(b))


def summarise(arms):
    out = ["caching sensitivity\n"]
    out.append("cells where the implicit cache activated\n")
    for key, project, strategy, n, total, tokens in cached_cells(arms):
        out.append("  %-11s %-10s %s  %d/%d runs  cached tokens %s\n"
                   % (key, project, strategy, n, total, tokens))

    out.append("\ncontext-construction overhead, seconds per request\n")
    for key in ARM_KEYS:
        for strategy in STRATEGIES:
            values = [v for (p, s), cell in arms[key].items() if s == strategy
                      for v in [statistics.mean(cell["overhead"])]]
            out.append("  %-11s %s  cell means %.4f to %.4f\n"
                       % (key, strategy, min(values), max(values)))

    out.append("\nS3 against S1, per cent cheaper\n")
    out.append("  %-11s %-10s %6s %10s %10s\n"
               % ("arm", "project", "LOC", "uncached", "adjusted"))
    for key in ARM_KEYS:
        cells = arms[key]
        projects = sorted({p for p, _ in cells}, key=lambda p: cells[(p, "S1")]["loc"])
        for project in projects:
            out.append("  %-11s %-10s %6d %10.2f %10.2f\n"
                       % (key, project, cells[(project, "S1")]["loc"],
                          _saving(cells, project, "uncached"),
                          _saving(cells, project, "adjusted")))

    out.append("\ntelemetry, cache-adjusted, S1 against S3\n")
    for key in ARM_KEYS:
        cells = arms[key]
        base = cells[("telemetry", "S1")]["adjusted"]
        variant = cells[("telemetry", "S3")]["adjusted"]
        _, p = stats.permutation_mean_diff(base, variant)
        lo, hi = stats.bootstrap_saving_ci(base, variant)
        out.append("  %-11s saving %+6.2f %%  95 %% CI [%+.1f, %+.1f]  "
                   "exact permutation p %.4f\n"
                   % (key, _saving(cells, "telemetry", "adjusted"), lo, hi, p))

    out.append("\ninterpolated zero crossing between scheduler and telemetry\n")
    for key in ARM_KEYS:
        for field in ("uncached", "adjusted"):
            point = crossing(arms[key], field, "scheduler", "telemetry")
            out.append("  %-11s %-9s %s\n"
                       % (key, field,
                          "none" if point is None else "%.0f LOC" % point))
    return "".join(out)


def table(arms):
    rows = []
    for key in ARM_KEYS:
        cells = arms[key]
        base = cells[("telemetry", "S1")]["adjusted"]
        variant = cells[("telemetry", "S3")]["adjusted"]
        _, p = stats.permutation_mean_diff(base, variant)
        lo, hi = stats.bootstrap_saving_ci(base, variant)
        rows.append("%s & $%+.1f$ & $%+.1f$ & $[%+.1f, %+.1f]$ & %.3f"
                    % (ARM_LABEL[key],
                       _saving(cells, "telemetry", "uncached"),
                       _saving(cells, "telemetry", "adjusted"),
                       lo, hi, p))
    out = ["\\begin{table}[htbp]\n\\centering\n\\small\n",
           "\\begin{tabular}{lrrrr}\n\\toprule\n",
           "Arm & List rates & Cache-adjusted & 95\\% interval & $p$ \\\\\n",
           "\\midrule\n"]
    for row in rows:
        out.append(row + " \\\\\n")
    out.append("\\bottomrule\n\\end{tabular}\n")
    out.append("\\caption{\\CaptionCaching}\n\\label{tab:caching}\n")
    out.append("\\end{table}\n")
    return "".join(out)


def write(target, arms):
    path = os.path.join(target, "ch5-tab-caching.tex")
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(table(arms))
    return path


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
        print("usage: python -m harness.caching --default DIR "
              "--suppressed DIR [--figures DIR]")
        return 2
    arms = build(paths)
    print(summarise(arms))
    if target:
        print("table      %s" % write(target, arms))
    return 0


if __name__ == "__main__":
    sys.exit(main())
