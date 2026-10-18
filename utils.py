import os
import glob

FILE_PATTERNS = ('*.h', '*.hpp', '*.cpp', '*.cxx', '*.cc')

FILE_HEADER = "//--- file: %s ---"


def collect_project_files(project_dir):
    collected = []
    seen = set()
    for pattern in FILE_PATTERNS:
        matches = glob.glob(os.path.join(project_dir, '**', pattern), recursive=True)
        for filepath in sorted(matches):
            if filepath in seen:
                continue
            seen.add(filepath)
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read()
            except Exception:
                continue
            collected.append((os.path.relpath(filepath, project_dir), content))
    return collected


def format_files(files):
    return "\n\n".join(
        "%s\n%s" % (FILE_HEADER % name, content) for name, content in files
    )


def get_project_code(project_dir):
    return format_files(collect_project_files(project_dir))
