#!/usr/bin/env python3
"""Generate default and minimal projects and verify they build and test.

Invoked by the template CI job:

    python tests/wizard/test_generated_matrix.py \
        --exe build/ninja-debug/bin/cproject \
        --template-root .
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path


def run(command: list[str], cwd: Path, timeout: int = 300) -> subprocess.CompletedProcess:
    result = subprocess.run(
        command,
        cwd=cwd,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        timeout=timeout,
        env=os.environ.copy(),
    )
    if result.returncode != 0:
        raise AssertionError(
            f"command failed ({result.returncode}): {' '.join(command)}\n"
            f"--- stdout ---\n{result.stdout}\n--- stderr ---\n{result.stderr}"
        )
    return result


def assert_text(path: Path, needle: str) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    assert needle in text, f"{path} does not contain {needle!r}"


def scenario(exe: Path, template_root: Path, work: Path, name: str, minimal: bool) -> None:
    output = work / ("minimal" if minimal else "default")
    command = [
        str(exe),
        "new",
        name,
        "--template-root",
        str(template_root),
        "--output",
        str(output),
        "--description",
        f"{name} probe",
        "--author",
        "Matrix Author",
        "--project-version",
        "7.8.9",
        "--yes",
        "--json",
    ]
    if minimal:
        command += [
            "--no-installable",
            "--no-subproject",
            "--no-ci",
            "--no-cli",
            "--no-ai-agent",
        ]
    result = run(command, work)
    payload = json.loads(result.stdout)
    assert payload["ok"] is True, payload
    assert payload["dryRun"] is False
    assert payload["project"]["name"] == name
    assert payload["project"]["version"] == "7.8.9"
    expected = {
        "installable": not minimal,
        "subproject": not minimal,
        "ci": not minimal,
        "cli": not minimal,
        "ai-agent": False,
    }
    assert payload["features"] == expected, payload["features"]

    project = output / name
    assert (project / "CMakeLists.txt").is_file()
    assert_text(project / "CMakeLists.txt", f"project({name}")
    assert_text(project / "CMakeLists.txt", "VERSION 7.8.9")
    assert_text(project / "CMakeLists.txt", f'DESCRIPTION "{name} probe"')
    assert_text(project / "CMakeLists.txt", "MIT")
    assert_text(project / "README.md", f"# {name}")
    assert_text(project / "README.md", f"{name} probe")
    assert_text(project / "README.md", "Matrix Author")
    assert_text(project / "CHANGELOG.md", "## 7.8.9")
    assert_text(project / "LICENSE", "MIT License")
    assert_text(project / "LICENSE", "Matrix Author")

    for forbidden in (
        "installer",
        "src/wizard",
        "tests/wizard",
        "cmake/Wizard.cmake",
        "cmake/PackagingMSI.cmake",
        "templates",
        "docs/maintainer",
        "docs/guides/release.md",
        "docs/guides/template.md",
    ):
        assert not (project / forbidden).exists(), f"template-only path leaked: {forbidden}"

    if minimal:
        for absent in (
            ".github",
            "src/main.c",
            "res",
            "tests/package_smoke",
            "tests/subproject_smoke",
            "cmake/Packaging.cmake",
            "cmake/InstallRules.cmake",
        ):
            assert not (project / absent).exists(), f"minimal project kept {absent}"
    else:
        assert (project / ".github/workflows/ci.yml").is_file()
        assert (project / "templates").exists() is False
        ci = (project / ".github/workflows/ci.yml").read_text(encoding="utf-8")
        assert "package-smoke" in ci
        assert "subproject-smoke" in ci
        assert "ninja-shared" in ci
        assert (project / "AGENTS.md").exists() is False

    run(["cmake", "--preset", "ninja-debug"], project)
    run(["cmake", "--build", "--preset", "ninja-debug"], project)
    run(["ctest", "--preset", "ninja-debug", "--output-on-failure"], project)
    print(f"OK: {name} ({'minimal' if minimal else 'default'}) generated and tested")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--template-root", type=Path, required=True)
    args = parser.parse_args(argv[1:])
    exe = args.exe.resolve()
    template_root = args.template_root.resolve()
    assert exe.is_file(), f"wizard executable not found: {exe}"
    with tempfile.TemporaryDirectory(prefix="cproject-generated-matrix-") as tmp:
        work = Path(tmp)
        scenario(exe, template_root, work, "DefaultProbe", minimal=False)
        scenario(exe, template_root, work, "MinimalProbe", minimal=True)
    print("OK: generated-project matrix passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
