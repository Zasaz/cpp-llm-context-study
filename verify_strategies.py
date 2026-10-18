import os
import shutil
import sys
import tempfile

from analyzer import context_strategies
from verify_extractor import SAMPLE

ACTIVE = "Player.cpp"


def build_project(files):
    root = tempfile.mkdtemp(prefix="strategy_check_")
    for name, content in files.items():
        path = os.path.join(root, name)
        parent = os.path.dirname(path)
        if parent and not os.path.exists(parent):
            os.makedirs(parent)
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(content)
    return root


def main():
    root = build_project(SAMPLE)
    try:
        print("project    : %d files, point of interest %s" % (len(SAMPLE), ACTIVE))
        print("versions   : strategies %s, extractor %s\n"
              % (context_strategies.STRATEGY_VERSION,
                 context_strategies.symbol_extractor.EXTRACTOR_VERSION))

        results = {}
        for name in context_strategies.VALID_STRATEGIES:
            context, meta = context_strategies.select_context(
                root, strategy=name, active_file=ACTIVE
            )
            results[name] = (context, meta)
            print("%s  %d/%d files, %d chars"
                  % (name, meta["files_selected"], meta["files_available"],
                     meta["context_chars"]))
            print("    selected: %s" % ", ".join(meta["files_selected_names"]))
            if name == context_strategies.S3:
                print("    seeds %s, symbols reached %s, max distance %s, cap applied %s"
                      % (meta.get("seed_symbols"), meta.get("symbols_reached"),
                         meta.get("max_distance"), meta.get("cap_applied")))
            if meta.get("degenerate_reason"):
                print("    DEGENERATE: %s" % meta["degenerate_reason"])

        s1 = results["S1"][1]["files_selected"]
        s2 = results["S2"][1]["files_selected"]
        s3 = results["S3"][1]["files_selected"]

        print("\nseparation check:")
        print("  S1 selects %d, S2 selects %d, S3 selects %d" % (s1, s2, s3))
        problems = []
        if s2 >= s1:
            problems.append("S2 is not narrower than S1")
        if s3 >= s1:
            problems.append("S3 is not narrower than S1 on this project")
        if s3 <= s2:
            problems.append("S3 reached nothing beyond the active file, so it is S2")
        for p in problems:
            print("  PROBLEM: %s" % p)
        if not problems:
            print("  the three conditions separate")
        return 1 if problems else 0
    finally:
        shutil.rmtree(root, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
