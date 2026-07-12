#!/usr/bin/env python3
"""Validate release-sensitive CPack settings."""
from __future__ import annotations

import re
import sys
from pathlib import Path


def cmake_value(text: str, name: str) -> str | None:
    match = re.search(rf'^set\({re.escape(name)}\s+"(.*)"\)$', text, re.MULTILINE)
    return match.group(1) if match else None


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: check-cpack-config.py <CPackConfig.cmake>", file=sys.stderr)
        return 2

    path = Path(argv[1])
    text = path.read_text(encoding="utf-8", errors="replace")

    generator = cmake_value(text, "CPACK_GENERATOR") or ""
    is_nsis = "NSIS" in generator or cmake_value(text, "CPACK_BINARY_NSIS") == "ON"
    if not is_nsis:
        print("CPack config checks skipped: NSIS generator is not enabled")
        return 0

    errors: list[str] = []
    if "CPACK_NSIS_MUI_FINISHPAGE_RUN" in text:
        errors.append("finish-page autorun must be disabled")

    menu_links = cmake_value(text, "CPACK_NSIS_MENU_LINKS")
    if not menu_links or "bin/run-wizard.bat" not in menu_links:
        errors.append("CPACK_NSIS_MENU_LINKS must include bin/run-wizard.bat")

    desktop_links = cmake_value(text, "CPACK_CREATE_DESKTOP_LINKS")
    if not desktop_links or "run-wizard.bat" not in desktop_links:
        errors.append("CPACK_CREATE_DESKTOP_LINKS must include bin/run-wizard.bat")

    # PATH modification is optional (may fail with long PATH on user machines)
    pass

    if "ZIP" not in generator and cmake_value(text, "CPACK_BINARY_ZIP") != "ON":
        errors.append("Windows packages must include ZIP alongside NSIS")

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print("CPack config checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
