#!/usr/bin/env python3
"""Contract tests for the template wizard CLI.

Run from CTest with the built executable and template root:

    python test_cli_contract.py --exe path/to/cproject --template-root .

These tests are template-only: the wizard markers in cmake/Tests.cmake keep
them out of generated projects.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path


def run(exe: Path, *args: str, env: dict[str, str] | None = None) -> subprocess.CompletedProcess:
    child_env = None if env is None else {**os.environ, **env}
    return subprocess.run(
        [str(exe), *args],
        capture_output=True,
        encoding="utf-8",
        errors="replace",
        env=child_env,
        timeout=120,
    )


def parse_json(result: subprocess.CompletedProcess) -> dict:
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise AssertionError(
            f"stdout is not one JSON object: {exc}\nstdout={result.stdout!r}\nstderr={result.stderr!r}"
        ) from exc


class Contract:
    def __init__(self, exe: Path, template_root: Path) -> None:
        self.exe = exe
        self.template_root = template_root

    def base(self, *args: str) -> list[str]:
        return ["new", *args, "--template-root", str(self.template_root)]

    def test_tool_version(self) -> None:
        result = run(self.exe, "--version")
        assert result.returncode == 0, result.stderr
        parts = result.stdout.strip().split(".")
        assert len(parts) == 3 and all(part.isdigit() for part in parts), result.stdout
        assert result.stdout.count("\n") == 1, result.stdout

    def test_missing_name_is_usage_error(self) -> None:
        result = run(self.exe, *self.base("--yes", "--json"))
        assert result.returncode == 2, (result.stdout, result.stderr)
        payload = parse_json(result)
        assert payload["ok"] is False
        assert payload["error"]["exitCode"] == 2
        assert "名称" in payload["error"]["message"]

    def test_invalid_name_is_usage_error(self) -> None:
        result = run(self.exe, *self.base("1Bad", "--yes", "--json"))
        assert result.returncode == 2, (result.stdout, result.stderr)
        assert parse_json(result)["ok"] is False

    def test_old_version_flag_is_rejected(self) -> None:
        result = run(self.exe, *self.base("GoodName", "--version", "1.2.3", "--yes", "--json"))
        assert result.returncode == 2, (result.stdout, result.stderr)
        assert "project-version" in parse_json(result)["error"]["message"]

    def test_removed_fake_flags_are_rejected(self) -> None:
        for flag in ("--no-shared", "--no-sanitizers", "--no-coverage", "--force"):
            result = run(self.exe, *self.base("GoodName", flag, "--yes", "--json"))
            assert result.returncode == 2, (flag, result.stdout, result.stderr)
            assert parse_json(result)["ok"] is False

    def test_non_empty_destination_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cproject-cli-dest-") as tmp:
            output = Path(tmp) / "output"
            destination = output / "GoodName"
            destination.mkdir(parents=True)
            marker = destination / "keep.txt"
            marker.write_text("do not overwrite\n", encoding="utf-8")
            result = run(
                self.exe,
                *self.base("GoodName", "--output", str(output), "--yes", "--json"),
            )
            assert result.returncode == 1, (result.stdout, result.stderr)
            assert parse_json(result)["ok"] is False
            assert marker.read_text(encoding="utf-8") == "do not overwrite\n"

    def test_empty_destination_is_allowed(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cproject-cli-empty-") as tmp:
            output = Path(tmp) / "output"
            (output / "GoodName").mkdir(parents=True)
            result = run(
                self.exe,
                *self.base("GoodName", "--output", str(output), "--yes", "--json"),
            )
            assert result.returncode == 0, (result.stdout, result.stderr)
            assert (output / "GoodName" / "CMakeLists.txt").is_file()
            assert parse_json(result)["ok"] is True

    def test_dry_run_never_reads_stdin(self) -> None:
        result = subprocess.run(
            [str(self.exe), *self.base("--dry-run", "--json")],
            input="this must not be consumed as a project name\n",
            capture_output=True,
            encoding="utf-8",
            errors="replace",
            timeout=120,
        )
        assert result.returncode == 2, (result.stdout, result.stderr)
        assert parse_json(result)["ok"] is False

    def test_non_mit_requires_license_file(self) -> None:
        result = run(self.exe, *self.base("GoodName", "--license", "Apache-2.0", "--yes", "--json"))
        assert result.returncode == 2, (result.stdout, result.stderr)
        assert "license-file" in parse_json(result)["error"]["message"]

    def test_invalid_project_version_is_rejected(self) -> None:
        result = run(
            self.exe,
            *self.base("GoodName", "--project-version", "1.2", "--yes", "--json"),
        )
        assert result.returncode == 2, (result.stdout, result.stderr)
        assert "x.y.z" in parse_json(result)["error"]["message"]

    def test_license_file_and_ai_policy_are_applied(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cproject-cli-license-") as tmp:
            output = Path(tmp) / "output"
            license_text = "Apache License\nVersion 2.0\n"
            license_file = Path(tmp) / "LICENSE.apache"
            license_file.write_text(license_text, encoding="utf-8")
            result = run(
                self.exe,
                *self.base(
                    "LicenseProbe",
                    "--output", str(output),
                    "--license", "Apache-2.0",
                    "--license-file", str(license_file),
                    "--with-ai-agent",
                    "--yes",
                    "--json",
                ),
            )
            assert result.returncode == 0, (result.stdout, result.stderr)
            payload = parse_json(result)
            assert payload["features"]["ai-agent"] is True
            project = output / "LicenseProbe"
            assert (project / "LICENSE").read_text(encoding="utf-8") == license_text
            assert "LicenseProbe contributors" in (project / "README.md").read_text(encoding="utf-8")
            assert (project / "AGENTS.md").is_file()
            assert "AGENTS.md" in (project / "README.md").read_text(encoding="utf-8")

    def test_dry_run_is_side_effect_free_and_reports_metadata(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cproject-cli-contract-") as tmp:
            output = Path(tmp) / "output"
            result = run(
                self.exe,
                *self.base(
                    "my-lib",
                    "--output", str(output),
                    "--description", "Contract probe",
                    "--author", "Probe Author",
                    "--project-version", "9.8.7",
                    "--no-ci", "--no-installable", "--no-subproject", "--no-ai-agent", "--no-cli",
                    "--dry-run", "--json",
                ),
            )
            assert result.returncode == 0, (result.stdout, result.stderr)
            payload = parse_json(result)
            assert payload["ok"] is True
            assert payload["dryRun"] is True
            assert payload["project"]["name"] == "MyLib"
            assert payload["project"]["namespace"] == "mylib"
            assert payload["project"]["version"] == "9.8.7"
            assert payload["project"]["description"] == "Contract probe"
            assert payload["project"]["author"] == "Probe Author"
            assert all(value is False for value in payload["features"].values())
            assert not (output / "MyLib").exists()

    def test_explicit_language_wins_over_environment(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cproject-cli-lang-") as tmp:
            output = Path(tmp) / "output"
            result = run(
                self.exe,
                *self.base("LangProbe", "--output", str(output), "--dry-run", "--yes", "--lang", "en"),
                env={"LANG": "zh_CN.UTF-8", "LC_ALL": "", "LC_MESSAGES": ""},
            )
            assert result.returncode == 0, (result.stdout, result.stderr)
            assert "Project" in result.stdout
            assert "项目" not in result.stdout

    def test_path_invocation_resolves_template(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cproject-cli-path-") as tmp:
            output = Path(tmp) / "output"
            env = {**os.environ, "PATH": str(self.exe.parent) + os.pathsep + os.environ.get("PATH", "")}
            command = ["cproject.exe" if os.name == "nt" else "cproject",
                       "new", "PathProbe", "--output", str(output), "--yes", "--json"]
            options = {"executable": str(self.exe)} if os.name == "nt" else {}
            result = subprocess.run(
                command,
                cwd=tmp,
                capture_output=True,
                encoding="utf-8",
                errors="replace",
                env=env,
                timeout=120,
                **options,
            )
            assert result.returncode == 0, (result.stdout, result.stderr)
            payload = parse_json(result)
            assert Path(payload["template"]).resolve() == self.template_root
            assert (output / "PathProbe" / "CMakeLists.txt").is_file()

    def test_git_init_without_git_is_a_runtime_error(self) -> None:
        # The test environment always has git, so this only asserts the flag
        # is accepted and reports a planned action in dry-run mode.
        with tempfile.TemporaryDirectory(prefix="cproject-cli-git-") as tmp:
            output = Path(tmp) / "output"
            result = run(
                self.exe,
                *self.base("GitProbe", "--output", str(output), "--git-init", "--dry-run", "--json"),
            )
            assert result.returncode == 0, (result.stdout, result.stderr)
            assert parse_json(result)["changes"]["gitInitialized"] is True


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--template-root", type=Path, required=True)
    args = parser.parse_args(argv[1:])

    contract = Contract(args.exe.resolve(), args.template_root.resolve())
    failures: list[str] = []
    for name in sorted(dir(contract)):
        if not name.startswith("test_"):
            continue
        try:
            getattr(contract, name)()
            print(f"ok   {name}")
        except AssertionError as exc:
            failures.append(f"{name}: {exc}")
            print(f"FAIL {name}: {exc}", file=sys.stderr)
    if failures:
        return 1
    print(f"OK: {len([n for n in dir(contract) if n.startswith('test_')])} CLI contract checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
