import subprocess
import os
import tempfile
import re
import shutil

SOURCE_EXTENSIONS = ('.cpp', '.cxx', '.cc')
HEADER_EXTENSIONS = ('.h', '.hpp')

COMPILE_TIMEOUT_S = 20
RUN_TIMEOUT_S = 15


def rename_main_in_code(code_content):
    pattern = re.compile(r"(\b(?:int|void)\s+)(main)(\s*\((?:[^)]*\))?\s*\{)")
    return pattern.sub(r"\1user_code_main\3", code_content)


def _collect_sources(root):
    found = []
    for dirpath, _dirnames, filenames in os.walk(root):
        for filename in sorted(filenames):
            if filename.endswith(SOURCE_EXTENSIONS):
                full = os.path.join(dirpath, filename)
                found.append(os.path.relpath(full, root))
    return sorted(found)


def _include_dirs(root):
    dirs = [root]
    for dirpath, _dirnames, filenames in os.walk(root):
        if dirpath == root:
            continue
        if any(f.endswith(HEADER_EXTENSIONS) for f in filenames):
            dirs.append(dirpath)
    return dirs


def run_test_suite(project_dir, test_suite_content):
    result = {
        "output": "",
        "compilation_log": "",
        "execution_error": "",
        "system_error": None,
        "compiled": False,
        "ran": False,
        "exit_code": None,
        "sources_compiled": 0,
    }
    log_parts = []

    try:
        with tempfile.TemporaryDirectory() as temp_dir:
            build_dir = os.path.join(temp_dir, "build")
            shutil.copytree(project_dir, build_dir)

            sources = _collect_sources(build_dir)
            log_parts.append(
                "--- Preparing user code (renaming main() to user_code_main) ---\n"
                "%d source files: %s" % (len(sources), ", ".join(sources))
            )
            for relative in sources:
                path = os.path.join(build_dir, relative)
                with open(path, "r", encoding="utf-8", errors="replace") as fh:
                    content = fh.read()
                with open(path, "w", encoding="utf-8") as fh:
                    fh.write(rename_main_in_code(content))

            test_suite_path = os.path.join(build_dir, "test_suite.cpp")
            with open(test_suite_path, "w", encoding="utf-8") as fh:
                fh.write(test_suite_content)

            executable_path = os.path.join(temp_dir, "test_executable")

            includes = []
            for directory in _include_dirs(build_dir):
                includes.extend(["-I", directory])

            files_to_compile = [os.path.join(build_dir, s) for s in sources]
            files_to_compile.append(test_suite_path)
            result["sources_compiled"] = len(files_to_compile)

            compile_command = (
                ["g++"] + includes + files_to_compile
                + ["-o", executable_path, "-std=c++17", "-Wall"]
            )

            compile_proc = subprocess.run(
                compile_command, capture_output=True, text=True,
                timeout=COMPILE_TIMEOUT_S
            )
            log_parts.append(
                "--- Test Suite Compilation & Linking ---\nCommand: %s\n"
                "STDOUT:\n%s\nSTDERR:\n%s"
                % (" ".join(compile_command), compile_proc.stdout, compile_proc.stderr)
            )
            result["compilation_log"] = "\n\n".join(log_parts)

            if compile_proc.returncode != 0:
                result["system_error"] = "Test suite compilation or linking failed."
                return result

            result["compiled"] = True

            run_proc = subprocess.run(
                [executable_path], capture_output=True, text=True,
                timeout=RUN_TIMEOUT_S
            )
            result["ran"] = True
            result["exit_code"] = run_proc.returncode
            result["output"] = run_proc.stdout
            if run_proc.stderr:
                result["execution_error"] = run_proc.stderr

            if run_proc.returncode != 0:
                result["system_error"] = (
                    (result.get("system_error") or "")
                    + "\nTest suite execution returned non-zero exit code: %d."
                    % run_proc.returncode
                ).strip()

    except subprocess.TimeoutExpired:
        result["system_error"] = "Compilation or execution timed out."
        result["compilation_log"] = "\n\n".join(log_parts)
    except FileNotFoundError as e:
        result["system_error"] = "Compiler (g++) or related tool not found: %s" % e
    except Exception as e:
        result["system_error"] = "An unexpected error occurred: %s" % e

    return result
