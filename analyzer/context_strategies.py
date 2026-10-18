import os
import re

from analyzer import symbol_extractor
from utils import collect_project_files, format_files

STRATEGY_VERSION = "1.0"

S1 = "S1"
S2 = "S2"
S3 = "S3"
S4 = "S4"
S5 = "S5"
DEFAULT_STRATEGY = S1

VALID_STRATEGIES = (S1, S2, S3, S4, S5)

_ALIASES = {
    "s1": S1, "whole_project": S1, "whole-project": S1, "concatenation": S1,
    "s2": S2, "active_file": S2, "editor": S2, "editor-scoped": S2,
    "s3": S3, "symbol_graph": S3, "structural": S3, "graph": S3,
    "s4": S4, "includes": S4, "direct_includes": S4, "heuristic": S4,
    "s5": S5, "include_closure": S5, "transitive_includes": S5,
}

DEFAULT_MAX_DEPTH = 2
DEFAULT_MAX_CHARS = 200000


def normalise_strategy(value):
    if not value:
        return DEFAULT_STRATEGY
    key = str(value).strip().lower()
    return _ALIASES.get(key, DEFAULT_STRATEGY)


def select_context(project_dir, strategy=None, active_file=None, selection=None,
                   max_depth=DEFAULT_MAX_DEPTH, max_chars=DEFAULT_MAX_CHARS):
    name = normalise_strategy(strategy)
    files = collect_project_files(project_dir)
    available = [n for n, _ in files]

    meta = {
        "strategy_requested": strategy,
        "strategy_applied": name,
        "strategy_version": STRATEGY_VERSION,
        "extractor_version": symbol_extractor.EXTRACTOR_VERSION,
        "files_available": len(files),
        "active_file": active_file,
        "has_selection": bool(selection and selection.strip()),
    }

    resolved_active = _resolve_active(active_file, available)
    meta["active_file_resolved"] = resolved_active

    if name == S2:
        chosen, extra = _select_active_only(files, resolved_active)
    elif name == S3:
        chosen, extra = _select_by_graph(
            files, resolved_active, selection, max_depth, max_chars
        )
    elif name == S4:
        chosen, extra = _select_by_includes(files, resolved_active, depth=1)
    elif name == S5:
        chosen, extra = _select_by_includes(files, resolved_active, depth=None)
    else:
        name = S1
        chosen, extra = list(files), {}

    meta.update(extra)
    meta["strategy_applied"] = name
    meta["files_selected"] = len(chosen)
    meta["files_selected_names"] = [n for n, _ in chosen]

    context = format_files(chosen)
    meta["context_chars"] = len(context)
    return context, meta


def _resolve_active(active_file, available):
    if not active_file:
        return None
    normalised = active_file.replace("\\", "/")
    for name in available:
        if name.replace("\\", "/") == normalised:
            return name
    base = os.path.basename(normalised)
    matches = [n for n in available if os.path.basename(n) == base]
    if len(matches) == 1:
        return matches[0]
    return matches[0] if matches else None


def _select_active_only(files, resolved_active):
    if resolved_active is None:
        return [], {"degenerate_reason": "no active file resolved"}
    chosen = [(n, c) for n, c in files if n == resolved_active]
    return chosen, {}


def _select_by_graph(files, resolved_active, selection, max_depth, max_chars):
    extra = {
        "traversal_depth": max_depth,
        "max_chars": max_chars,
        "cap_applied": False,
    }
    if resolved_active is None:
        return [], dict(extra, degenerate_reason="no active file resolved")

    project_code = {n: c for n, c in files}
    symbols = symbol_extractor.extract_symbols_from_files(project_code)

    seeds = sorted(n for n, s in symbols.items() if s.get("file") == resolved_active)
    if selection and selection.strip():
        narrowed = _seeds_in_selection(symbols, resolved_active, selection,
                                       project_code.get(resolved_active, ""))
        if narrowed:
            seeds = narrowed

    extra["seed_symbols"] = len(seeds)
    if not seeds:
        extra["degenerate_reason"] = "no symbols extracted from the active file"

    methods_of = {}
    for symbol_name, record in symbols.items():
        owner = record.get("owner")
        if owner:
            methods_of.setdefault(owner, []).append(symbol_name)

    distance_of_file = {resolved_active: 0}
    visited = set(seeds)
    frontier = [(s, 0) for s in seeds]

    while frontier:
        next_frontier = []
        for name, depth in frontier:
            record = symbols.get(name)
            if record is None:
                continue
            holder = record.get("file")
            if holder is not None and depth < distance_of_file.get(holder, 10 ** 9):
                distance_of_file[holder] = depth
            if depth >= max_depth:
                continue
            neighbours = list(record.get("calls", [])) + list(record.get("bases", []))
            if record.get("owner"):
                neighbours.append(record["owner"])
            neighbours.extend(methods_of.get(name, []))
            for target in sorted(set(neighbours)):
                if target in visited or target not in symbols:
                    continue
                visited.add(target)
                next_frontier.append((target, depth + 1))
        frontier = next_frontier

    extra["symbols_reached"] = len(visited)

    reached = sorted(distance_of_file, key=lambda n: (distance_of_file[n], n))
    contents = {n: c for n, c in files}

    chosen = []
    total = 0
    evicted = []
    for name in reached:
        content = contents.get(name)
        if content is None:
            continue
        cost = len(content) + len(name) + 32
        if chosen and max_chars is not None and total + cost > max_chars:
            evicted.append(name)
            continue
        chosen.append((name, content))
        total += cost

    if evicted:
        extra["cap_applied"] = True
        extra["files_evicted"] = len(evicted)
        extra["files_evicted_names"] = evicted

    order = {n: i for i, (n, _) in enumerate(files)}
    chosen.sort(key=lambda pair: order.get(pair[0], 10 ** 9))
    extra["max_distance"] = max(
        (distance_of_file[n] for n, _ in chosen), default=None
    )
    return chosen, extra


def _seeds_in_selection(symbols, active_file, selection, content):
    if not content:
        return []
    index = content.find(selection.strip().split("\n")[0])
    if index < 0:
        return []
    first_line = content.count("\n", 0, index) + 1
    last_line = first_line + selection.strip().count("\n")
    hits = []
    for name, record in symbols.items():
        if record.get("file") != active_file:
            continue
        start = record.get("line_start")
        end = record.get("line_end", start)
        if start is None:
            continue
        if start <= last_line and (end or start) >= first_line:
            hits.append(name)
    return sorted(hits)


_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)

_SOURCE_SUFFIXES = (".cpp", ".cc", ".cxx")


def _select_by_includes(files, resolved_active, depth=1):
    """A deliberately cheap alternative to the symbol graph.

    Follows quoted includes from the active file, adding for each header the
    implementation file sharing its stem. With depth 1 only the active file's own
    includes are followed, which is the baseline as literally specified. With
    depth None the include relation is followed to closure, which is the stronger
    version and the one that actually competes with a symbol graph. Neither parses
    C++, resolves a type or builds a graph.
    """
    extra = {
        "seed_symbols": None,
        "selection_rule": "include closure, depth %s" % (depth or "unbounded"),
    }
    if resolved_active is None:
        return [], dict(extra, degenerate_reason="no active file resolved")

    contents = {name: code for name, code in files}
    available = list(contents)

    def stem(name):
        return os.path.splitext(os.path.basename(name))[0]

    def resolve(included):
        base = os.path.basename(included)
        for candidate in available:
            if os.path.basename(candidate) == base:
                return candidate
        return None

    def partners(name):
        return [c for c in available
                if stem(c) == stem(name) and c.endswith(_SOURCE_SUFFIXES)]

    wanted = {resolved_active}
    frontier = [(resolved_active, 0)]
    seen = {resolved_active}
    while frontier:
        nxt = []
        for name, level in frontier:
            if depth is not None and level >= depth:
                continue
            for included in _INCLUDE_RE.findall(contents.get(name, "")):
                target = resolve(included)
                if target is None:
                    continue
                for member in [target] + partners(target):
                    wanted.add(member)
                    if member not in seen:
                        seen.add(member)
                        nxt.append((member, level + 1))
        frontier = nxt

    for candidate in partners(resolved_active):
        wanted.add(candidate)

    chosen = [(name, code) for name, code in files if name in wanted]
    extra["symbols_reached"] = None
    extra["files_followed"] = len(wanted) - 1
    return chosen, extra
