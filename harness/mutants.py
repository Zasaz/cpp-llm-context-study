"""Seeded-fault analysis.

Answers a question the campaigns do not: given that a generated suite compiles
and runs against the correct project, does it detect a defect introduced into
that project? Mutants are generated mechanically from the corpus, kept only if
they still compile, and every archived suite that succeeded on the clean project
is replayed against every surviving mutant. No model call is made, so this costs
nothing and uses the suites the campaigns already produced.
"""

import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

CXX = os.environ.get("CXX", "g++")
STD = "-std=c++17"
COMPILE_TIMEOUT = 40
RUN_TIMEOUT = 20

OPERATORS = (
    ("relational", (
        (r"(?<![<>=!])<(?![<=])", "<="),
        (r"(?<![<>=!])>(?![>=])", ">="),
        (r"<=", "<"),
        (r">=", ">"),
        (r"==", "!="),
    )),
    ("arithmetic", (
        (r"(?<![+\-*/=<>!+])\+(?![+=])", "-"),
        (r"(?<![+\-*/=<>!\-])\-(?![\-=>])", "+"),
        (r"(?<![*/=])\*(?![*=])", "/"),
    )),
    ("boundary", (
        (r"\b0\b", "1"),
        (r"\b1\b", "0"),
    )),
)


def _sources(projdir):
    return sorted(n for n in os.listdir(projdir) if n.endswith(".cpp"))


def _guard(line):
    stripped = line.strip()
    if stripped.startswith("#") or stripped.startswith("//"):
        return True
    return "for (" in line and "++" in line


def generate(projdir, limit, only=None):
    """Yield (id, file, line_number, before, after, kind) candidate mutants.

    ``only`` restricts seeding to one file. The task prompts the model to test
    the active file, so a fault seeded anywhere else measures whether the suite
    happens to exercise unrelated code rather than whether it detects a defect
    in what it was asked to test.
    """
    out = []
    names = [only] if only else _sources(projdir)
    for name in names:
        path = os.path.join(projdir, name)
        with open(path, encoding="utf-8") as fh:
            lines = fh.read().split("\n")
        for number, line in enumerate(lines):
            if _guard(line):
                continue
            for kind, rules in OPERATORS:
                for pattern, replacement in rules:
                    match = re.search(pattern, line)
                    if not match:
                        continue
                    mutated = line[:match.start()] + replacement + line[match.end():]
                    if mutated == line:
                        continue
                    out.append({
                        "file": name, "line": number,
                        "before": line, "after": mutated, "kind": kind,
                    })
                    break
    # spread the sample over files and operators rather than taking a prefix
    out.sort(key=lambda m: (m["file"], m["line"]))
    if len(out) <= limit:
        chosen = out
    else:
        step = len(out) / float(limit)
        chosen = [out[int(i * step)] for i in range(limit)]
    for index, mutant in enumerate(chosen, 1):
        mutant["id"] = "m%02d" % index
    return chosen


def materialise(projdir, mutant, target):
    shutil.copytree(projdir, target, dirs_exist_ok=True)
    path = os.path.join(target, mutant["file"])
    with open(path, encoding="utf-8") as fh:
        lines = fh.read().split("\n")
    lines[mutant["line"]] = mutant["after"]
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))


def _rename_main(code):
    pattern = re.compile(r"(\b(?:int|void)\s+)(main)(\s*\((?:[^)]*\))?\s*\{)")
    return pattern.sub(r"\1user_code_main\3", code)


def build_and_run(projdir, suite_code, workdir):
    shutil.copytree(projdir, workdir, dirs_exist_ok=True)
    for name in _sources(projdir):
        path = os.path.join(workdir, name)
        with open(path, encoding="utf-8") as fh:
            code = fh.read()
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(_rename_main(code))
    suite_path = os.path.join(workdir, "generated_suite.cpp")
    with open(suite_path, "w", encoding="utf-8") as fh:
        fh.write(suite_code)
    binary = os.path.join(workdir, "suite.bin")
    sources = [os.path.join(workdir, n) for n in _sources(projdir)] + [suite_path]
    try:
        compiled = subprocess.run(
            [CXX, STD, "-I", workdir, "-o", binary] + sources,
            capture_output=True, text=True, timeout=COMPILE_TIMEOUT)
    except subprocess.TimeoutExpired:
        return {"compiled": False, "ran": False, "exit_code": None}
    if compiled.returncode != 0:
        return {"compiled": False, "ran": False, "exit_code": None}
    try:
        ran = subprocess.run([binary], capture_output=True, text=True,
                             timeout=RUN_TIMEOUT)
    except subprocess.TimeoutExpired:
        return {"compiled": True, "ran": True, "exit_code": "timeout"}
    return {"compiled": True, "ran": True, "exit_code": ran.returncode}


def load_suites(campaign, project, strategy):
    records = []
    path = os.path.join(campaign, "results.jsonl")
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            record = json.loads(line)
            if record["project"] != project or record["strategy"] != strategy:
                continue
            if not (record.get("grade") or {}).get("fully_successful"):
                continue
            suite = os.path.join(campaign, "artefacts",
                                 record["artefact_prefix"] + ".suite.cpp")
            if os.path.exists(suite):
                with open(suite, encoding="utf-8") as sfh:
                    records.append((record["artefact_prefix"], sfh.read()))
    return records


def run(corpus, campaign, strategies, per_project, max_suites,
        active_only=True):
    with open(os.path.join(corpus, "manifest.json"), encoding="utf-8") as fh:
        manifest = json.load(fh)
    results = []
    for entry in manifest["projects"]:
        project = entry["name"]
        projdir = os.path.join(corpus, entry["dir"])
        candidates = generate(projdir, per_project * 4,
                              only=entry["active_file"] if active_only else None)

        viable = []
        for mutant in candidates:
            with tempfile.TemporaryDirectory() as tmp:
                target = os.path.join(tmp, "p")
                materialise(projdir, mutant, target)
                probe = build_and_run(target, "int main(){return 0;}\n",
                                      os.path.join(tmp, "w"))
            if probe["compiled"]:
                viable.append(mutant)
            if len(viable) >= per_project:
                break
        sys.stderr.write("%-10s %d viable mutants\n" % (project, len(viable)))

        for strategy in strategies:
            suites = load_suites(campaign, project, strategy)[:max_suites]
            for mutant in viable:
                killed = 0
                ran = 0
                for prefix, code in suites:
                    with tempfile.TemporaryDirectory() as tmp:
                        target = os.path.join(tmp, "p")
                        materialise(projdir, mutant, target)
                        outcome = build_and_run(target, code,
                                                os.path.join(tmp, "w"))
                    if not outcome["compiled"]:
                        continue
                    ran += 1
                    if outcome["exit_code"] != 0:
                        killed += 1
                results.append({
                    "project": project, "strategy": strategy,
                    "mutant": mutant["id"], "kind": mutant["kind"],
                    "file": mutant["file"], "line": mutant["line"] + 1,
                    "suites_run": ran, "suites_killed": killed,
                })
            sys.stderr.write("  %s %s done\n" % (project, strategy))
    return results


def report(rows):
    out = ["seeded-fault detection, suites replayed against compiling mutants\n\n"]
    strategies = sorted({r["strategy"] for r in rows})
    out.append("  %-10s %s\n" % ("project", "  ".join("%-16s" % s for s in strategies)))
    projects = sorted({r["project"] for r in rows})
    totals = {s: [0, 0] for s in strategies}
    for project in projects:
        cells = []
        for strategy in strategies:
            sel = [r for r in rows
                   if r["project"] == project and r["strategy"] == strategy]
            killed = sum(r["suites_killed"] for r in sel)
            ran = sum(r["suites_run"] for r in sel)
            totals[strategy][0] += killed
            totals[strategy][1] += ran
            cells.append("%-16s" % ("%d/%d  %5.1f%%" % (
                killed, ran, 100.0 * killed / ran if ran else 0.0)))
        out.append("  %-10s %s\n" % (project, "  ".join(cells)))
    cells = []
    for strategy in strategies:
        killed, ran = totals[strategy]
        cells.append("%-16s" % ("%d/%d  %5.1f%%" % (
            killed, ran, 100.0 * killed / ran if ran else 0.0)))
    out.append("  %-10s %s\n" % ("all", "  ".join(cells)))
    return "".join(out)


def table(rows):
    strategies = sorted({r["strategy"] for r in rows})
    projects = sorted({r["project"] for r in rows})
    body = []
    for project in projects:
        cells = []
        for strategy in strategies:
            sel = [r for r in rows
                   if r["project"] == project and r["strategy"] == strategy]
            killed = sum(r["suites_killed"] for r in sel)
            ran = sum(r["suites_run"] for r in sel)
            cells.append("--- & ---" if not ran
                         else "%d/%d & %.1f" % (killed, ran, 100.0 * killed / ran))
        body.append("%s & %s" % (project, " & ".join(cells)))
    cells = []
    for strategy in strategies:
        killed = sum(r["suites_killed"] for r in rows if r["strategy"] == strategy)
        ran = sum(r["suites_run"] for r in rows if r["strategy"] == strategy)
        cells.append("%d/%d & %.1f" % (killed, ran, 100.0 * killed / ran))
    spec = "l" + "rr" * len(strategies)
    head = " & ".join("\\multicolumn{2}{c}{%s}" % s for s in strategies)
    sub = " & ".join("Detected & \\%" for _s in strategies)
    out = ["\\begin{table}[htbp]\n\\centering\n\\small\n",
           "\\begin{tabular}{%s}\n\\toprule\n" % spec,
           "& %s \\\\\n" % head,
           "Project & %s \\\\\n\\midrule\n" % sub]
    for row in body:
        out.append(row + " \\\\\n")
    out.append("\\midrule\n")
    out.append("All & " + " & ".join(cells) + " \\\\\n")
    out.append("\\bottomrule\n\\end{tabular}\n")
    out.append("\\caption{\\CaptionMutants}\n\\label{tab:mutants}\n")
    out.append("\\end{table}\n")
    return "".join(out)


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)

    def opt(flag, default):
        return argv[argv.index(flag) + 1] if flag in argv else default

    corpus = opt("--corpus", "corpus")
    campaign = opt("--campaign", None)
    if campaign is None:
        print("usage: python -m harness.mutants --campaign DIR "
              "[--corpus DIR] [--mutants N] [--suites N] [--json PATH]")
        return 2
    strategies = opt("--strategies", "S1,S2,S3").split(",")
    rows = run(corpus, campaign, strategies,
               int(opt("--mutants", "5")), int(opt("--suites", "10")),
               active_only="--all-files" not in argv)
    print(report(rows))
    if "--figures" in argv:
        path = os.path.join(opt("--figures", "."), "ch5-tab-mutants.tex")
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(table(rows))
        print("wrote %s" % path)
    if "--json" in argv:
        with open(opt("--json", ""), "w", encoding="utf-8") as fh:
            json.dump(rows, fh, indent=2)
    return 0


if __name__ == "__main__":
    sys.exit(main())
