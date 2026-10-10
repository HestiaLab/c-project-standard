#!/usr/bin/env python3
"""Unit tests for the repository's quality-gate scripts (pure stdlib).

These tests assert that the gates can actually fail — a gate that cannot
produce a nonzero exit code on broken input is not a gate. Each test drives
the real script as a subprocess (or imports it) against throwaway fixtures
under a temporary directory; the repository itself is never modified.

Run directly:
    python3 scripts/tests/test_gate_behaviors.py
Or via unittest discovery:
    python3 -m unittest discover -s scripts/tests
"""

import importlib.util
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

# scripts/tests/ -> scripts/ -> repo root
_REPO_ROOT = Path(__file__).resolve().parents[2]
_SCRIPTS_DIR = _REPO_ROOT / "scripts"


def _run_script(script: Path, *args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        [sys.executable, str(script), *args],
        capture_output=True,
        text=True,
        timeout=120,
    )


def _load_module(script: Path, module_name: str):
    spec = importlib.util.spec_from_file_location(module_name, script)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestDeclDefGate(unittest.TestCase):
    """scripts/check_decl_def.py must pass clean trees and fail broken ones."""

    def setUp(self) -> None:
        self.script = _SCRIPTS_DIR / "check_decl_def.py"
        self.assertTrue(self.script.is_file(), f"missing gate script: {self.script}")
        self.tmp = Path(tempfile.mkdtemp(prefix="gate-test-decldef-"))
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)

    def test_clean_repo_passes(self) -> None:
        result = _run_script(self.script, str(_REPO_ROOT))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("OK:", result.stdout)

    def test_empty_scan_surface_fails(self) -> None:
        """A gate that scans nothing must not report OK (scan-surface floor)."""
        empty = self.tmp / "empty"
        empty.mkdir()
        result = _run_script(self.script, str(empty))
        self.assertEqual(result.returncode, 1)
        self.assertIn("解析失败", result.stderr)

    def test_explicit_floor_is_enforced(self) -> None:
        """An explicit floor above the real scan surface must fail.

        This is what the template-strict registration (inside the wizard
        markers) relies on; it must hold for any tree, not just this repo.
        """
        result = _run_script(self.script, str(_REPO_ROOT),
                             "--min-declarations", "100000")
        self.assertEqual(result.returncode, 1)
        self.assertIn("下限 100000", result.stderr)

    def test_floor_at_or_below_surface_passes(self) -> None:
        result = _run_script(self.script, str(_REPO_ROOT),
                             "--min-declarations", "1",
                             "--min-definitions", "1")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_missing_definition_fails_and_names_location(self) -> None:
        """Removing one definition from a copy of the tree must fail, naming
        the header and the function."""
        work = self.tmp / "repo"
        shutil.copytree(_REPO_ROOT / "include", work / "include")
        shutil.copytree(_REPO_ROOT / "src", work / "src")

        # Drop the definition body of the first public function defined in
        # src/<ns>/example.c (implementation of the example public header).
        example_sources = sorted((work / "src").rglob("example.c"))
        self.assertTrue(example_sources, "fixture expects src/**/example.c")
        target = example_sources[0]
        text = target.read_text(encoding="utf-8")
        # Find the first public function definition (same shape the gate
        # matches: `name(...) {`) and delete the whole function, from the
        # start of its line (return type) to its closing brace.
        import re
        match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{",
                          text)
        assert match is not None, "fixture expects a top-level definition"
        start = text.rfind("\n", 0, match.start()) + 1
        depth = 0
        end = start
        for i in range(match.end() - 1, len(text)):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    end = i + 1
                    break
        patched = text[:start] + text[end:]
        target.write_text(patched, encoding="utf-8")

        result = _run_script(self.script, str(work))
        self.assertEqual(result.returncode, 1)
        self.assertIn("public declaration without definition", result.stderr)


class TestVersionConsistencyGate(unittest.TestCase):
    """scripts/check_version_consistency.py parse helpers must be exact."""

    def setUp(self) -> None:
        script = _SCRIPTS_DIR / "check_version_consistency.py"
        self.assertTrue(script.is_file(), f"missing gate script: {script}")
        self.module = _load_module(script, "check_version_consistency")
        self.tmp = Path(tempfile.mkdtemp(prefix="gate-test-version-"))
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)

    def test_parses_project_version(self) -> None:
        cmakelists = self.tmp / "CMakeLists.txt"
        cmakelists.write_text(
            "cmake_minimum_required(VERSION 3.21)\n"
            "project(SomeName\n    VERSION 1.2.3\n    LANGUAGES C\n)\n",
            encoding="utf-8",
        )
        parsed = self.module.read_cmake_version(cmakelists)
        self.assertEqual(parsed, ("SomeName", "1.2.3"))

    def test_rejects_missing_project_version(self) -> None:
        cmakelists = self.tmp / "CMakeLists.txt"
        cmakelists.write_text("cmake_minimum_required(VERSION 3.21)\n",
                              encoding="utf-8")
        self.assertIsNone(self.module.read_cmake_version(cmakelists))

    def test_parses_newest_changelog_section(self) -> None:
        changelog = self.tmp / "CHANGELOG.md"
        changelog.write_text(
            "# 更新日志\n\n## 0.3.0 — 2026-01-01\n\n- x\n\n## 0.2.0\n\n- y\n",
            encoding="utf-8",
        )
        self.assertEqual(self.module.read_changelog_version(changelog), "0.3.0")

    def test_reads_generated_header(self) -> None:
        header = self.tmp / "version.h"
        header.write_text(
            '#define SOMEPROJ_VERSION_STRING "4.5.6"\n', encoding="utf-8")
        self.assertEqual(self.module.read_header_version(header, "SomeProj"),
                         "4.5.6")

    def test_missing_inputs_exit_nonzero(self) -> None:
        """Pointing the gate at an empty tree must fail, not pass vacuously."""
        empty = self.tmp / "empty"
        empty.mkdir()
        result = _run_script(
            _SCRIPTS_DIR / "check_version_consistency.py",
            str(empty),
            "--build-dir", str(empty),
            "--exe", str(empty / "no-such-exe"),
        )
        self.assertEqual(result.returncode, 1)


class TestCpackConfigGate(unittest.TestCase):
    """scripts/check-cpack-config.py must reject a missing config file."""

    def test_missing_config_fails(self) -> None:
        script = _SCRIPTS_DIR / "check-cpack-config.py"
        self.assertTrue(script.is_file(), f"missing gate script: {script}")
        with tempfile.TemporaryDirectory(prefix="gate-test-cpack-") as tmp:
            result = _run_script(script, str(Path(tmp) / "CPackConfig.cmake"))
            self.assertEqual(result.returncode, 1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
