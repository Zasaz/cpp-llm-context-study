import json
import os

from analyzer import symbol_extractor
from utils import collect_project_files


class Project(object):
    def __init__(self, name, path, active_file, files):
        self.name = name
        self.path = path
        self.active_file = active_file
        self.files = files
        self.metrics = self._measure()

    def _measure(self):
        code = {name: content for name, content in self.files}
        symbols = symbol_extractor.extract_symbols_from_files(code)

        loc = 0
        for _name, content in self.files:
            for line in content.split("\n"):
                if line.strip():
                    loc += 1

        call_edges = sum(len(s.get("calls", [])) for s in symbols.values())
        classes = sum(1 for s in symbols.values() if s["type"] == "class")
        functions = sum(1 for s in symbols.values() if s["type"] == "function")

        return {
            "files": len(self.files),
            "loc": loc,
            "chars": sum(len(c) for _n, c in self.files),
            "symbols": len(symbols),
            "classes": classes,
            "functions": functions,
            "call_edges": call_edges,
        }

    def as_payload(self):
        return [{"name": name, "content": content} for name, content in self.files]

    def __repr__(self):
        return "<Project %s files=%d loc=%d symbols=%d>" % (
            self.name, self.metrics["files"], self.metrics["loc"],
            self.metrics["symbols"],
        )


def load_corpus(corpus_dir):
    manifest_path = os.path.join(corpus_dir, "manifest.json")
    with open(manifest_path, "r", encoding="utf-8") as fh:
        manifest = json.load(fh)

    projects = []
    for entry in manifest["projects"]:
        path = os.path.join(corpus_dir, entry["dir"])
        if not os.path.isdir(path):
            raise ValueError("corpus project directory missing: %s" % path)
        files = collect_project_files(path)
        if not files:
            raise ValueError("corpus project has no C++ files: %s" % path)

        active = entry.get("active_file")
        names = [n for n, _c in files]
        if active and active not in names:
            raise ValueError(
                "active_file %r not present in project %s (has %s)"
                % (active, entry["name"], ", ".join(names))
            )

        projects.append(Project(entry["name"], path, active, files))

    projects.sort(key=lambda p: (p.metrics["loc"], p.name))
    return projects
