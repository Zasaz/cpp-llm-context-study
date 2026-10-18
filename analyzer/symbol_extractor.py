import re

EXTRACTOR_VERSION = "2.1-typed"

_KEYWORDS = {
    "if", "while", "for", "switch", "catch", "sizeof", "return", "else",
    "do", "new", "delete", "throw", "case", "static_cast", "dynamic_cast",
    "const_cast", "reinterpret_cast", "assert", "and", "or", "not",
}

_DECL_RE = re.compile(
    r"(?:^|[;{}:])\s*(?:const\s+|static\s+|mutable\s+)*"
    r"([A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)*)"
    r"\s*(<[^;{}()]*>)?\s*[*&]?\s*"
    r"([A-Za-z_]\w*)\s*(?=[;=({\[])"
)

_CLASS_RE = re.compile(r"\b(class|struct)\s+([A-Za-z_]\w*)\s*(?::([^{;]*))?\{")
_BASE_RE = re.compile(r"(?:public|private|protected|virtual)?\s*([A-Za-z_]\w*(?:::\w+)*)")

_CALL_NAME_RE = re.compile(r"\b([A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)*)\s*\(")

_RECEIVER_TAIL_RE = re.compile(
    r"([A-Za-z_]\w*(?:\s*\[[^\[\]]*\])?"
    r"(?:\s*(?:\.|->)\s*[A-Za-z_]\w*(?:\s*\[[^\[\]]*\])?)*)"
    r"\s*(?:\.|->)\s*$"
)

_RECEIVER_WINDOW = 200


def strip_comments_and_strings(text):
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        two = text[i:i + 2]
        if two == "//":
            while i < n and text[i] != "\n":
                out.append(" ")
                i += 1
        elif two == "/*":
            while i < n and text[i:i + 2] != "*/":
                out.append("\n" if text[i] == "\n" else " ")
                i += 1
            out.append("  ")
            i = min(i + 2, n)
        elif c in "\"'":
            quote = c
            out.append(" ")
            i += 1
            while i < n and text[i] != quote:
                if text[i] == "\\":
                    out.append(" ")
                    i += 1
                    if i < n:
                        out.append(" ")
                        i += 1
                    continue
                out.append("\n" if text[i] == "\n" else " ")
                i += 1
            if i < n:
                out.append(" ")
                i += 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def _match_brace(text, open_index):
    depth = 0
    for i in range(open_index, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i
    return -1


def _line_of(text, index):
    return text.count("\n", 0, index) + 1


def _find_classes(clean):
    found = []
    for m in _CLASS_RE.finditer(clean):
        name = m.group(2)
        bases = []
        if m.group(3):
            for bm in _BASE_RE.finditer(m.group(3)):
                base = bm.group(1)
                if base not in ("public", "private", "protected", "virtual"):
                    bases.append(base)
        open_index = m.end() - 1
        close_index = _match_brace(clean, open_index)
        if close_index == -1:
            continue
        found.append((name, bases, open_index + 1, close_index, m.start()))
    return found


def _resolve_declared_type(type_name, template_args, known_classes):
    simple = type_name.split("::")[-1]
    if type_name in known_classes:
        return type_name
    if simple in known_classes:
        return simple
    if template_args:
        for candidate in re.findall(r"[A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)*", template_args):
            candidate = re.sub(r"\s+", "", candidate).split("::")[-1]
            if candidate in known_classes:
                return candidate
    return None


def _declarations_in(clean, span, known_classes):
    start, end = span
    region = clean[start:end]
    out = {}
    for m in _DECL_RE.finditer(region):
        type_name = re.sub(r"\s+", "", m.group(1))
        var_name = m.group(3)
        if type_name in _KEYWORDS or var_name in _KEYWORDS:
            continue
        resolved = _resolve_declared_type(type_name, m.group(2), known_classes)
        if resolved:
            out[var_name] = resolved
    return out


def _find_functions(clean, known_classes, class_spans):
    results = []
    pattern = re.compile(
        r"(~?)\s*([A-Za-z_]\w*)\s*(?:::\s*(~?)\s*([A-Za-z_]\w*)\s*)?\([^;{}()]*\)\s*"
        r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:->[^{;]*)?\{"
    )
    for m in pattern.finditer(clean):
        first, second = m.group(2), m.group(4)
        tilde = "~" if (m.group(1) or m.group(3)) else ""
        if first in _KEYWORDS or (second and second in _KEYWORDS):
            continue
        head_start = clean.rfind("\n", 0, m.start()) + 1
        head = clean[head_start:m.start()]
        if re.search(r"\b(class|struct|enum|namespace)\b", head):
            continue

        open_index = m.end() - 1
        close_index = _match_brace(clean, open_index)
        if close_index == -1:
            continue

        if second:
            qualified = "%s::%s%s" % (first, tilde, second)
            owner = first if first in known_classes else None
        else:
            owner = None
            for cname, _bases, cstart, cend, _hidx in class_spans:
                if cstart <= m.start() < cend:
                    if owner is None or cstart > 0:
                        owner = cname
            if owner:
                qualified = "%s::%s%s" % (owner, tilde, first)
            elif tilde:
                qualified = "%s::~%s" % (first, first)
                owner = first if first in known_classes else None
            else:
                qualified = first

        results.append((qualified, owner, open_index + 1, close_index, m.start()))
    return results


def extract_symbols_from_files(project_code):
    symbols = {}
    cleaned = {}
    class_info = {}

    for filename, content in project_code.items():
        cleaned[filename] = strip_comments_and_strings(content)

    for filename, clean in cleaned.items():
        for name, bases, body_start, body_end, header_index in _find_classes(clean):
            class_info[name] = {
                "file": filename,
                "bases": bases,
                "span": (body_start, body_end),
            }
            symbols[name] = {
                "file": filename,
                "type": "class",
                "line_start": _line_of(clean, header_index),
                "line_end": _line_of(clean, body_end),
                "bases": bases,
            }

    known_classes = set(class_info)

    class_members = {}
    for name, info in class_info.items():
        clean = cleaned[info["file"]]
        class_members[name] = _declarations_in(clean, info["span"], known_classes)

    functions = {}
    for filename, clean in cleaned.items():
        class_spans = [
            (n, i["bases"], i["span"][0], i["span"][1], 0)
            for n, i in class_info.items()
            if i["file"] == filename
        ]
        for qualified, owner, body_start, body_end, header_index in _find_functions(
            clean, known_classes, class_spans
        ):
            record = {
                "file": filename,
                "type": "function",
                "line_start": _line_of(clean, header_index),
                "line_end": _line_of(clean, body_end),
                "body": clean[header_index:body_end + 1],
                "owner": owner,
                "span": (body_start, body_end),
            }
            if symbols.get(qualified, {}).get("type") == "class":
                continue
            functions[qualified] = record
            symbols[qualified] = record

    for qualified, record in functions.items():
        clean = cleaned[record["file"]]
        scope = {}
        owner = record.get("owner")
        if owner:
            scope.update(_inherited_members(owner, class_info, class_members))
        scope.update(_declarations_in(clean, record["span"], known_classes))

        body = clean[record["span"][0]:record["span"][1]]
        calls = []
        seen = set()
        for m in _CALL_NAME_RE.finditer(body):
            called = re.sub(r"\s+", "", m.group(1))
            if called in _KEYWORDS or called in known_classes:
                continue
            if called.split("::")[-1] in _KEYWORDS:
                continue

            window = body[max(0, m.start() - _RECEIVER_WINDOW):m.start()]
            receiver_match = _RECEIVER_TAIL_RE.search(window)

            target = None
            if receiver_match:
                receiver_class = _resolve_receiver(
                    _receiver_segments(receiver_match.group(1)),
                    scope, class_members, class_info, owner,
                )
                target = _lookup_method(
                    receiver_class, called, class_info, functions
                )
            elif "::" in called and called in functions:
                target = called
            elif called in functions:
                target = called
            elif owner:
                target = _lookup_method(owner, called, class_info, functions)

            if target and target != qualified and target not in seen:
                seen.add(target)
                calls.append(target)

        record["calls"] = calls

    for record in symbols.values():
        record.pop("span", None)

    return symbols


def _receiver_segments(expr):
    segments = []
    for part in re.split(r"\.|->", expr):
        part = re.sub(r"\[[^\[\]]*\]", "", part).strip()
        if part:
            segments.append(part)
    return segments


def _resolve_receiver(segments, scope, class_members, class_info, owner):
    if not segments:
        return None

    head = segments[0]
    if head == "this":
        current = owner
    elif head in scope:
        current = scope[head]
    elif head in class_info:
        current = head
    else:
        return None

    for segment in segments[1:]:
        if not current:
            return None
        fields = _inherited_members(current, class_info, class_members)
        current = fields.get(segment)
    return current


def _inherited_members(class_name, class_info, class_members, _depth=0):
    if _depth > 8 or class_name not in class_info:
        return {}
    out = {}
    for base in class_info[class_name]["bases"]:
        out.update(_inherited_members(base, class_info, class_members, _depth + 1))
    out.update(class_members.get(class_name, {}))
    return out


def _lookup_method(class_name, method, class_info, functions, _depth=0):
    if not class_name or _depth > 8:
        return None
    candidate = "%s::%s" % (class_name, method)
    if candidate in functions:
        return candidate
    for base in class_info.get(class_name, {}).get("bases", []):
        found = _lookup_method(base, method, class_info, functions, _depth + 1)
        if found:
            return found
    return None
