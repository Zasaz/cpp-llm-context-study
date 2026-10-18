import argparse
import json
import random
import os
import platform
import subprocess
import sys
import time

from harness import HARNESS_VERSION
from harness.corpus import load_corpus
from harness.grade import grade

PRICING = {
    "model": "gemini-3.7-flash",
    "input_per_million_usd": 0.75,
    "output_per_million_usd": 3.75,
    "note": "output rate includes thinking tokens",
    "read_on": "2026-08-20",
    "valid_until": "2026-12-31",
}

GENERATION_PARAMS = {
    "temperature": 1.0,
    "top_p": 0.95,
    "candidate_count": 1,
}

UNCONTROLLED_PARAMS = {
    "thinking": "left at the service default, which performs thinking. It is "
                "settable: types.ThinkingConfig(thinking_budget=0) suppresses it "
                "in google-genai 1.47.0, verified 2026-08-21. Campaigns before "
                "that date did not set it.",
}

TASKS = ("generate_tests",)


def cost_usd(prompt_tokens, output_tokens, thought_tokens):
    if prompt_tokens is None and output_tokens is None:
        return None
    billed_output = (output_tokens or 0) + (thought_tokens or 0)
    return (
        (prompt_tokens or 0) * PRICING["input_per_million_usd"] / 1_000_000.0
        + billed_output * PRICING["output_per_million_usd"] / 1_000_000.0
    )


def _write(path, text):
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(text if text is not None else "")


def _package_version(name):
    try:
        from importlib import metadata
    except ImportError:
        return None
    try:
        return metadata.version(name)
    except Exception:
        return None


def _compiler_version():
    try:
        proc = subprocess.run(["g++", "--version"], capture_output=True,
                              text=True, timeout=10)
    except Exception:
        return None
    if proc.returncode != 0:
        return None
    return (proc.stdout or "").strip().splitlines()[0] if proc.stdout else None


def environment():
    return {
        "python": platform.python_version(),
        "platform": platform.platform(),
        "machine": platform.machine(),
        "google_genai": _package_version("google-genai"),
        "flask": _package_version("flask"),
        "compiler": _compiler_version(),
        "compile_flags": ["-std=c++17", "-Wall"],
    }


def run_campaign(corpus_dir, out_dir, strategies, repetitions, project_filter=None,
                 dry_run=False, thinking=True, shuffle_seed=None):
    import gemini_integration.client as client
    from analyzer import context_strategies, symbol_extractor
    import main as service

    client.RETAIN_TEXT = True
    params = dict(GENERATION_PARAMS)
    if not thinking:
        params["thinking_config"] = {"thinking_budget": 0}
    client.GENERATION_PARAMS = params

    projects = load_corpus(corpus_dir)
    if project_filter:
        wanted = set(project_filter)
        projects = [p for p in projects if p.name in wanted]
        if not projects:
            raise ValueError("no corpus project matched %s" % sorted(wanted))

    stamp = time.strftime("%Y%m%dT%H%M%S")
    run_dir = os.path.join(out_dir, "campaign_%s" % stamp)
    artefact_dir = os.path.join(run_dir, "artefacts")
    os.makedirs(artefact_dir)

    manifest = {
        "started": stamp,
        "harness_version": HARNESS_VERSION,
        "strategy_version": context_strategies.STRATEGY_VERSION,
        "extractor_version": symbol_extractor.EXTRACTOR_VERSION,
        "model_requested": client.MODEL_NAME,
        "environment": environment(),
        "generation_params": params,
        "thinking": "default" if thinking else "suppressed",
        "uncontrolled_params": UNCONTROLLED_PARAMS if thinking else {},
        "pricing": PRICING,
        "repetitions": repetitions,
        "run_order": ("shuffled, seed %d" % shuffle_seed)
                     if shuffle_seed is not None
                     else "fixed nested, project/task/strategy/repetition",
        "strategies": list(strategies),
        "tasks": list(TASKS),
        "dry_run": dry_run,
        "projects": [
            {"name": p.name, "active_file": p.active_file, "metrics": p.metrics}
            for p in projects
        ],
    }
    _write(os.path.join(run_dir, "manifest.json"), json.dumps(manifest, indent=2))

    print("campaign %s" % run_dir)
    print("  model %s, %d project(s), %d strateg(ies), %d repetition(s)"
          % (client.MODEL_NAME, len(projects), len(strategies), repetitions))
    for p in projects:
        print("    %-12s files=%-3d loc=%-5d symbols=%-4d call_edges=%d"
              % (p.name, p.metrics["files"], p.metrics["loc"],
                 p.metrics["symbols"], p.metrics["call_edges"]))
    total_runs = len(projects) * len(TASKS) * len(strategies) * repetitions
    print("  %d runs%s\n" % (total_runs, " (dry)" if dry_run else ""))

    app = service.app.test_client()
    results_path = os.path.join(run_dir, "results.jsonl")
    results = []
    index = 0

    schedule = [
        (project, task, strategy, repetition)
        for project in projects
        for task in TASKS
        for strategy in strategies
        for repetition in range(1, repetitions + 1)
    ]
    if shuffle_seed is not None:
        random.Random(shuffle_seed).shuffle(schedule)

    payloads = {}
    with open(results_path, "w", encoding="utf-8") as results_fh:
        for project, task, strategy, repetition in schedule:
            if project.name not in payloads:
                payloads[project.name] = project.as_payload()
            payload_files = payloads[project.name]
            index += 1
            label = "%s/%s/%s/r%d" % (
                project.name, task, strategy, repetition
            )
            record = _run_one(
                app, client, project, payload_files, task, strategy,
                repetition, artefact_dir, index, dry_run
            )
            results.append(record)
            results_fh.write(json.dumps(record) + "\n")
            results_fh.flush()
            _print_line(label, record)

    _write(os.path.join(run_dir, "summary.json"),
           json.dumps(summarise(results), indent=2))
    print("\nresults  %s" % results_path)
    print("summary  %s" % os.path.join(run_dir, "summary.json"))
    return run_dir, results


def _run_one(app, client, project, payload_files, task, strategy, repetition,
             artefact_dir, index, dry_run):
    record = {
        "index": index,
        "project": project.name,
        "project_metrics": project.metrics,
        "task": task,
        "strategy": strategy,
        "repetition": repetition,
        "active_file": project.active_file,
        "timestamp": time.time(),
    }

    client.reset_call_log()

    generate_body = {
        "feature": task,
        "files": payload_files,
        "active_file": project.active_file,
        "strategy": strategy,
    }

    if dry_run:
        from analyzer import context_strategies
        import shutil
        import tempfile

        temp = tempfile.mkdtemp()
        try:
            for item in payload_files:
                path = os.path.join(temp, item["name"])
                parent = os.path.dirname(path)
                if parent and not os.path.exists(parent):
                    os.makedirs(parent)
                _write(path, item["content"])
            _context, meta = context_strategies.select_context(
                temp, strategy=strategy, active_file=project.active_file
            )
        finally:
            shutil.rmtree(temp, ignore_errors=True)
        record["context_meta"] = meta
        record["dry_run"] = True
        return record

    started_request = time.perf_counter()
    response = app.post("/api/analyze", json=generate_body)
    record["request_latency_s"] = time.perf_counter() - started_request
    record["http_status"] = response.status_code

    body = response.get_json() or {}
    record["context_meta"] = body.get("context_meta")

    if response.status_code != 200 or "generated_test_suite_code" not in body:
        record["error"] = body.get("error") or "no suite in response"
        record["grade"] = grade("", None, None)
        return record

    suite = body["generated_test_suite_code"]

    log = client.get_call_log()
    entry = log[-1] if log else {}
    record["model_call"] = {
        "model_requested": entry.get("model_requested"),
        "model_returned": entry.get("model_returned"),
        "prompt_tokens": entry.get("prompt_tokens"),
        "output_tokens": entry.get("output_tokens"),
        "thought_tokens": entry.get("thought_tokens"),
        "cached_tokens": entry.get("cached_tokens"),
        "total_tokens": entry.get("total_tokens"),
        "model_latency_s": entry.get("latency_s"),
        "generation_params": entry.get("generation_params"),
        "ok": entry.get("ok"),
        "error": entry.get("error"),
    }
    record["cost_usd"] = cost_usd(
        entry.get("prompt_tokens"), entry.get("output_tokens"),
        entry.get("thought_tokens")
    )
    if record.get("request_latency_s") is not None and entry.get("latency_s"):
        record["overhead_latency_s"] = (
            record["request_latency_s"] - entry["latency_s"]
        )

    started_exec = time.perf_counter()
    execute_body = {
        "feature": "run_test_suite",
        "files": payload_files,
        "test_suite_code": suite,
    }
    exec_response = app.post("/api/analyze", json=execute_body)
    record["execution_latency_s"] = time.perf_counter() - started_exec
    run_result = exec_response.get_json() or {}

    record["grade"] = grade(suite, entry.get("response"), run_result)

    prefix = "%04d_%s_%s_%s_r%d" % (
        index, project.name, task, strategy, repetition
    )
    _write(os.path.join(artefact_dir, prefix + ".prompt.txt"), entry.get("prompt"))
    _write(os.path.join(artefact_dir, prefix + ".response.txt"), entry.get("response"))
    _write(os.path.join(artefact_dir, prefix + ".suite.cpp"), suite)
    _write(os.path.join(artefact_dir, prefix + ".compile.log"),
           run_result.get("compilation_log"))
    _write(os.path.join(artefact_dir, prefix + ".stdout.txt"),
           run_result.get("output"))
    record["artefact_prefix"] = prefix
    return record


def _print_line(label, record):
    if record.get("dry_run"):
        meta = record.get("context_meta") or {}
        print("  %-34s files=%s chars=%s"
              % (label, meta.get("files_selected"), meta.get("context_chars")))
        return
    g = record.get("grade") or {}
    call = record.get("model_call") or {}
    meta = record.get("context_meta") or {}
    if g.get("refused"):
        verdict = "REFUSED"
    elif g.get("fully_successful"):
        verdict = "ok"
    elif g.get("failure_kind") == "crashed":
        verdict = "CRASH"
    elif g.get("failure_kind") == "assertion_failed":
        verdict = "assert!"
    elif g.get("compiled"):
        verdict = "compiled"
    else:
        verdict = "FAILED"
    print("  %-34s %-9s files=%-2s ptok=%-6s think=%-5s %.2fs $%.5f"
          % (label, verdict, meta.get("files_selected"),
             call.get("prompt_tokens"), call.get("thought_tokens"),
             call.get("model_latency_s") or 0.0, record.get("cost_usd") or 0.0))


def summarise(results):
    buckets = {}
    for record in results:
        if record.get("dry_run"):
            continue
        key = (record["project"], record["strategy"])
        buckets.setdefault(key, []).append(record)

    summary = []
    for (project, strategy), records in sorted(buckets.items()):
        n = len(records)
        grades = [r.get("grade") or {} for r in records]
        calls = [r.get("model_call") or {} for r in records]

        def mean(values):
            values = [v for v in values if isinstance(v, (int, float))]
            return sum(values) / len(values) if values else None

        summary.append({
            "project": project,
            "strategy": strategy,
            "runs": n,
            "files_selected": (records[0].get("context_meta") or {}).get(
                "files_selected"
            ),
            "compiled": sum(1 for g in grades if g.get("compiled")),
            "ran_clean": sum(1 for g in grades if g.get("ran_clean")),
            "asserted": sum(1 for g in grades if g.get("asserted")),
            "fully_successful": sum(1 for g in grades if g.get("fully_successful")),
            "refused": sum(1 for g in grades if g.get("refused")),
            "crashed": sum(1 for g in grades if g.get("failure_kind") == "crashed"),
            "assertion_failed": sum(
                1 for g in grades if g.get("failure_kind") == "assertion_failed"
            ),
            "mean_prompt_tokens": mean([c.get("prompt_tokens") for c in calls]),
            "mean_output_tokens": mean([c.get("output_tokens") for c in calls]),
            "mean_thought_tokens": mean([c.get("thought_tokens") for c in calls]),
            "mean_model_latency_s": mean([c.get("model_latency_s") for c in calls]),
            "mean_request_latency_s": mean(
                [r.get("request_latency_s") for r in records]
            ),
            "mean_cost_usd": mean([r.get("cost_usd") for r in records]),
        })
    return {"pricing": PRICING, "by_condition": summary}


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--corpus", default="corpus")
    parser.add_argument("--out", default="results")
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument("--strategies", default="S1,S2,S3")
    parser.add_argument("--projects", default=None)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--no-thinking", action="store_true")
    parser.add_argument("--shuffle-seed", type=int, default=None)
    args = parser.parse_args(argv)

    strategies = [s.strip() for s in args.strategies.split(",") if s.strip()]
    projects = (
        [p.strip() for p in args.projects.split(",") if p.strip()]
        if args.projects else None
    )

    run_campaign(
        corpus_dir=args.corpus,
        out_dir=args.out,
        strategies=strategies,
        repetitions=args.repetitions,
        project_filter=projects,
        dry_run=args.dry_run,
        thinking=not args.no_thinking,
        shuffle_seed=args.shuffle_seed,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
