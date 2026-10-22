#!/usr/bin/env python3
"""Verify staged install payload exclusions and the installed wizard.

Usage:
    python tests/wizard/test_install_payload.py \
        --build-dir build/ninja-debug --template-root .
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path


FORBIDDEN = (
    "installer",
    "src/wizard",
    "tests/wizard",
    "cmake/Wizard.cmake",
    "cmake/GenerateWiXTemplateTree.cmake",
    "cmake/PackagingMSI.cmake",
    "docs/maintainer",
    "scripts/check_changelog_tag.py",
    "scripts/check_doc_paths.py",
    "scripts/package.ps1",
    "scripts/publish.ps1",
    "scripts/gate_floor_probe.sh",
    "scripts/msi-smoke.ps1",
    ".github/workflows/release.yml",
    "CHANGELOG.md",
    "LICENSE",
    "src/main.c",
)


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


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--template-root", type=Path, required=True)
    args = parser.parse_args(argv[1:])
    build_dir = args.build_dir.resolve()
    template_root = args.template_root.resolve()

    with tempfile.TemporaryDirectory(prefix="cproject-install-payload-") as tmp:
        work = Path(tmp)
        prefix = work / "install"
        run(["cmake", "--install", str(build_dir), "--prefix", str(prefix)], template_root)
        share = prefix / "share/c-project-standard"
        assert share.is_dir(), "installed template tree is missing"
        for relative in FORBIDDEN:
            assert not (share / relative).exists(), f"template-only path leaked: {relative}"
        assert (share / "templates/ci.yml").is_file()
        assert (share / "template-exclude.txt").is_file()
        assert (share / "docs/adr/README.md").is_file()

        exe_name = "cproject.exe" if os.name == "nt" else "cproject"
        wizard = prefix / "bin" / exe_name
        assert wizard.is_file(), f"installed wizard missing: {wizard}"

        output = work / "generated"
        installed = run(
            [str(wizard), "new", "InstallProbe", "--output", str(output), "--yes", "--json"],
            work,
        )
        payload = json.loads(installed.stdout)
        assert payload["ok"] is True, payload
        resolved = Path(payload["template"]).resolve()
        assert resolved == share.resolve(), f"wizard used {resolved}, expected {share.resolve()}"
        project = output / "InstallProbe"
        assert (project / "CMakeLists.txt").is_file()
        assert not (project / "templates").exists()
        assert not (project / "docs/maintainer").exists()

        run(["cmake", "--preset", "ninja-debug"], project)
        run(["cmake", "--build", "--preset", "ninja-debug"], project)
        run(["ctest", "--preset", "ninja-debug", "--output-on-failure"], project)
        print("OK: install payload and installed wizard passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
