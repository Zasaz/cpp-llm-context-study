import re
from gemini_integration.client import call_gemini_api


def clean_response(raw):
    cleaned = (raw or "").strip()
    fenced = re.search(r'```[A-Za-z+]*\s*\n(.*?)```', cleaned, re.DOTALL)
    if fenced:
        return fenced.group(1).strip()
    cleaned = re.sub(r'^```[A-Za-z+]*\s*\n?', '', cleaned)
    return re.sub(r'\n?```\s*$', '', cleaned).strip()


def generate_tests(code_content, focus_filename=None):
    focus_instruction = ""
    if focus_filename:
        focus_instruction = f"Your primary goal is to generate comprehensive tests for the functions and classes defined in the file `{focus_filename}`. Use the other provided files for context to understand dependencies and usage."

    prompt = f"""
    You are an expert C++ unit testing assistant.
    Given the following C++ code project, generate a complete, compilable C++ unit test suite in a single code block.
    {focus_instruction}

    The test suite you generate MUST:
    1.  Include necessary standard headers (like `<cassert>`, `<iostream>`, `<vector>`, `<string>`, etc., as needed for the tests).
    2.  Create several distinct test functions. Each test function should focus on testing a specific aspect, function, or edge case. Name test functions clearly (e.g., `test_functionName_caseDescription()`).
    3.  Each test function MUST use `assert()` for checks.
    4.  Each test function should print a distinct success message to `std::cout` if all its assertions pass (e.g., `std::cout << "Test test_functionName_caseDescription: PASS" << std::endl;`).
    5.  Include YOUR OWN `main()` function in the test suite. This `main()` function will be the ONLY entry point for the test executable. It should call all the individual test functions you defined.
    6.  The test suite should be self-contained in terms of test logic.

    **Important for Compilation & Usability:**
    -   The user's C++ code (provided below) will be compiled first, and then this test suite will be compiled and linked against it. The test suite is compiled inside the project directory, so the project's own headers are on the include path.
    -   Therefore, you MUST obtain the declarations you need by including the project's own headers by name, for example `#include "Canvas.h"`.
    -   Do NOT re-declare or re-define any class, struct or function that the project's own headers already declare. A hand-written declaration that does not match the real one compiles and links without any diagnostic and then corrupts memory at run time.
    -   Do NOT attempt to call or interact with any `main()` function that might be present in the User's C++ Code.

    User's C++ Project Code:
    ```cpp
    {code_content}
    ```

    Generate the complete C++ unit test suite now as a single C++ code block.
    """
    try:
        test_suite_code_raw = call_gemini_api(prompt)

        return {"generated_test_suite_code": clean_response(test_suite_code_raw)}
    except Exception as e:
        return {"error": f"Failed to generate unit tests: {e}"}