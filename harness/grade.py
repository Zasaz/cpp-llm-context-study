import re

_SIGABRT = (-6, 134)
_SIGSEGV = (-11, 139)

_TEST_FUNCTION_RE = re.compile(
    r"^\s*(?:static\s+)?(?:void|int|bool)\s+(test\w*)\s*\(", re.MULTILINE
)
_ASSERT_RE = re.compile(r"\bassert\s*\(")
_PASS_RE = re.compile(r"\bPASS\b")

_REFUSAL_MARKERS = (
    "could not generate a response",
    "i cannot",
    "i'm unable",
    "i am unable",
    "as an ai",
)


def looks_like_refusal(raw_response, suite_code):
    if raw_response is None:
        return True
    lowered = raw_response.strip().lower()
    for marker in _REFUSAL_MARKERS:
        if marker in lowered:
            return True
    if suite_code and "#include" not in suite_code and "(" not in suite_code:
        return True
    return not (suite_code or "").strip()


def describe_suite(suite_code):
    code = suite_code or ""
    functions = _TEST_FUNCTION_RE.findall(code)
    return {
        "suite_chars": len(code),
        "test_functions": len(functions),
        "test_function_names": functions,
        "assertions": len(_ASSERT_RE.findall(code)),
        "has_main": bool(re.search(r"\bint\s+main\s*\(", code)),
    }


def grade(suite_code, raw_response, run_result):
    static = describe_suite(suite_code)
    refused = looks_like_refusal(raw_response, suite_code)

    output = (run_result or {}).get("output") or ""
    pass_lines = len(_PASS_RE.findall(output))

    compiled = bool((run_result or {}).get("compiled"))
    ran = bool((run_result or {}).get("ran"))
    exit_code = (run_result or {}).get("exit_code")
    ran_clean = bool(ran and exit_code == 0)

    asserted = bool(
        static["assertions"] > 0
        and static["test_functions"] > 0
        and pass_lines >= static["test_functions"]
    )

    if not ran or ran_clean:
        failure_kind = None
    elif exit_code in _SIGABRT:
        failure_kind = "assertion_failed"
    elif exit_code in _SIGSEGV:
        failure_kind = "crashed"
    else:
        failure_kind = "nonzero_exit"

    return {
        "refused": refused,
        "compiled": compiled and not refused,
        "ran": ran,
        "ran_clean": ran_clean,
        "exit_code": exit_code,
        "failure_kind": failure_kind,
        "pass_lines": pass_lines,
        "asserted": asserted,
        "fully_successful": bool(compiled and ran_clean and asserted and not refused),
        "compile_error": (run_result or {}).get("system_error"),
        **static,
    }
