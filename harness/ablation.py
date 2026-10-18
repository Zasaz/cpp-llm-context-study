"""Reference-graph ablation.

Runs the production S3 selection twice over each corpus project, once over the
graph the regex extractor produces and once over a graph built by a compiler
frontend, and reports whether the two ever select different files. The selection
code is the same in both cases: the extractor is substituted underneath it so
that nothing but the graph changes.
"""

import json
import os
import sys

from analyzer import context_strategies, symbol_extractor

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import verify_fidelity as fidelity  # noqa: E402

from clang import cindex  # noqa: E402


def _relative(cursor, projdir):
    location = cursor.location
    if location is None or location.file is None:
        return None
    path = os.path.abspath(location.file.name)
    root = os.path.abspath(projdir) + os.sep
    return path[len(root):] if path.startswith(root) else None


def reference_symbols(projdir, extra_args):
    names = set(fidelity.project_files(projdir))
    index = cindex.Index.create()
    tus = []
    for name in sorted(names):
        if not name.endswith(".cpp"):
            continue
        tu = index.parse(os.path.join(projdir, name),
                         args=["-std=c++17", "-I", projdir] + extra_args)
        for diag in tu.diagnostics:
            if diag.severity >= cindex.Diagnostic.Error:
                raise SystemExit("libclang error in %s: %s"
                                 % (name, diag.spelling))
        tus.append(tu)

    defs, files, owners = {}, {}, {}
    classes, bases = set(), {}

    def walk_defs(cursor):
        if cursor.kind in fidelity.DEF_KINDS and cursor.is_definition() \
                and fidelity.in_project(cursor, names):
            name = fidelity.qualified(cursor)
            defs[cursor.get_usr()] = name
            rel = _relative(cursor, projdir)
            if rel:
                files[name] = rel
            parent = cursor.semantic_parent
            if parent is not None and parent.kind in fidelity.CLASS_KINDS:
                owners[name] = parent.spelling
        if cursor.kind in fidelity.CLASS_KINDS and cursor.is_definition() \
                and fidelity.in_project(cursor, names):
            classes.add(cursor.spelling)
            rel = _relative(cursor, projdir)
            if rel:
                files.setdefault(cursor.spelling, rel)
            for child in cursor.get_children():
                if child.kind == cindex.CursorKind.CXX_BASE_SPECIFIER:
                    decl = child.type.get_declaration()
                    if decl is not None and fidelity.in_project(decl, names):
                        bases.setdefault(cursor.spelling, []).append(
                            decl.spelling)
        for child in cursor.get_children():
            walk_defs(child)

    for tu in tus:
        walk_defs(tu.cursor)

    edges = set()
    for tu in tus:
        fidelity.collect_calls(tu.cursor, names, defs, edges, None)

    symbols = {}
    for name in set(defs.values()):
        symbols[name] = {
            "type": "function",
            "file": files.get(name),
            "calls": sorted(t for s, t in edges if s == name),
            "bases": [],
            "owner": owners.get(name),
        }
    for name in classes:
        symbols[name] = {
            "type": "class",
            "file": files.get(name),
            "calls": [],
            "bases": sorted(bases.get(name, [])),
            "owner": None,
        }
    return {n: r for n, r in symbols.items() if r["file"]}


def select(projdir, active, symbols=None):
    if symbols is None:
        return context_strategies.select_context(projdir, "S3", active, "")
    original = symbol_extractor.extract_symbols_from_files
    symbol_extractor.extract_symbols_from_files = lambda _code: symbols
    try:
        return context_strategies.select_context(projdir, "S3", active, "")
    finally:
        symbol_extractor.extract_symbols_from_files = original


def run(corpus):
    with open(os.path.join(corpus, "manifest.json"), encoding="utf-8") as fh:
        manifest = json.load(fh)
    extra = fidelity.resource_args()
    rows = []
    for entry in manifest["projects"]:
        projdir = os.path.join(corpus, entry["dir"])
        active = entry["active_file"]

        _ext_ctx, ext_meta = select(projdir, active)
        _ref_ctx, ref_meta = select(projdir, active,
                                    reference_symbols(projdir, extra))

        ext_names = sorted(ext_meta["files_selected_names"])
        ref_names = sorted(ref_meta["files_selected_names"])
        ext_chars = ext_meta["context_chars"]
        ref_chars = ref_meta["context_chars"]
        rows.append({
            "project": entry["name"],
            "extractor_files": ext_names,
            "reference_files": ref_names,
            "identical": ext_names == ref_names,
            "only_reference": sorted(set(ref_names) - set(ext_names)),
            "only_extractor": sorted(set(ext_names) - set(ref_names)),
            "extractor_chars": ext_chars,
            "reference_chars": ref_chars,
            "extractor_reached": ext_meta.get("symbols_reached"),
            "reference_reached": ref_meta.get("symbols_reached"),
        })
    return rows


def report(rows):
    out = ["reference-graph ablation: does the fidelity gap change any prompt?\n"]
    out.append("  %-10s %-9s %-9s %-9s %s\n"
               % ("project", "ext files", "ref files", "identical", "chars ext / ref"))
    for r in rows:
        out.append("  %-10s %-9d %-9d %-9s %d / %d\n"
                   % (r["project"], len(r["extractor_files"]),
                      len(r["reference_files"]),
                      "yes" if r["identical"] else "NO",
                      r["extractor_chars"], r["reference_chars"]))
        if not r["identical"]:
            out.append("      reference only: %s\n"
                       % ", ".join(r["only_reference"]))
            out.append("      extractor only: %s\n"
                       % ", ".join(r["only_extractor"]))
    differing = [r for r in rows if not r["identical"]]
    out.append("\n  %d of %d projects select a different file set.\n"
               % (len(differing), len(rows)))
    return "".join(out)


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    corpus = argv[argv.index("--corpus") + 1] if "--corpus" in argv else "corpus"
    rows = run(corpus)
    print(report(rows))
    if "--json" in argv:
        path = argv[argv.index("--json") + 1]
        with open(path, "w", encoding="utf-8") as fh:
            json.dump(rows, fh, indent=2)
        print("wrote %s" % path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
