#!/usr/bin/env python3
"""Check that every public function declaration in include/ has a definition in src/.

Purpose
-------
Prevents the "declared but never defined" fake-API regression: a public header
that promises a function the implementation does not provide. The setup wizard
leaves a stub behind, and later edits can drop a definition while leaving its
declaration in place; this gate catches that drift.

Usage
-----
    python3 scripts/check_decl_def.py <repo_root>

Exit code: 0 = clean; 1 = at least one public declaration has no definition.

Approach
--------
Only declarations of the form ``name(...);`` in ``include/**/*.h`` are checked.
To keep the scan robust against real C headers, the text is pre-processed
before matching:

- comments (block and line) and string/char literals are blanked out, so a
  function name that appears inside a comment or ``"Use cproject_version()"``
  string is not mistaken for a declaration;
- all-uppercase identifiers are blanked out. By this project's naming convention
  (documented in ``docs/en/guides/coding-style.md``) functions and variables are
  snake_case and macros/constants are UPPER_SNAKE_CASE, so an all-uppercase name
  such as ``CPROJECT_DEPRECATED(...)`` is a macro, not a declared function;
- C11 keywords (plus a few common macro builtins) are skipped.

This makes the gate independent of the project's exact prefix, so it keeps
working after ``scripts/rename-project.*`` renames the template.

``typedef``s, macros, and ``extern`` variable declarations are ignored because
they do not match the ``name(...)`` shape. ``static inline`` functions defined
in a header end in ``{``, not ``;``, so they are not treated as bare
declarations. A declaration is matched to any definition of the same name in
``src/**/*.c``; signature equivalence is the compiler's job, not this gate's.
"""

import re
import sys
from pathlib import Path

# C11 keywords (plus common macro builtins) that must never be treated as a
# declared or defined function name, e.g. _Static_assert(sizeof(x) == 4, "...");
# or a control-flow statement such as while (x) { ... } in an implementation.
_KEYWORDS = frozenset(
    {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "inline", "int", "long", "register", "restrict", "return", "short",
        "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
        "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof",
        "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn",
        "_Static_assert", "_Thread_local", "offsetof",
    }
)

# A function declaration: `name(...)` terminated by `;` (multi-line aware).
_DECL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*;")
# A function definition: `name(...)` terminated by `{` (multi-line aware).
_DEF_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{")
# An all-uppercase identifier: by convention this is a macro or constant.
_MACRO_RE = re.compile(r"\b[A-Z][A-Z0-9_]*\b")


def _blank_span(chars: list[str], start: int, end: int) -> None:
    """Blank ``chars[start:end]`` with spaces, preserving newlines."""
    for i in range(start, end):
        if chars[i] != "\n":
            chars[i] = " "


def _strip_non_code(text: str) -> str:
    """Blank comment and string/char-literal contents, keeping newlines."""
    chars = list(text)
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            _blank_span(chars, i, end)
            i = end
        elif c == "/" and i + 1 < n and text[i + 1] == "/":
            end = text.find("\n", i + 2)
            end = n if end < 0 else end
            _blank_span(chars, i, end)
            i = end
        elif c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                j += 1
            j = min(j + 1, n)
            _blank_span(chars, i, j)
            i = j
        elif c == "'":
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == "\\":
                    j += 1
                j += 1
            j = min(j + 1, n)
            _blank_span(chars, i, j)
            i = j
        else:
            i += 1
    return "".join(chars)


def _code_text(path: Path) -> str:
    """Return the file text with comments, literals, and macros blanked out."""
    text = path.read_text(encoding="utf-8", errors="replace")
    text = _strip_non_code(text)
    return _MACRO_RE.sub(lambda match: " " * len(match.group(0)), text)


def collect_declarations(root: Path) -> dict[str, list[str]]:
    """Map each declared function name to the headers that declare it."""
    declarations: dict[str, list[str]] = {}
    for header in (root / "include").rglob("*.h"):
        for match in _DECL_RE.finditer(_code_text(header)):
            name = match.group(1)
            if name in _KEYWORDS:
                continue
            declarations.setdefault(name, []).append(str(header.relative_to(root)))
    return declarations


def collect_definitions(root: Path) -> set[str]:
    """Return the set of function names defined somewhere under src/."""
    definitions: set[str] = set()
    for source in (root / "src").rglob("*.c"):
        for match in _DEF_RE.finditer(_code_text(source)):
            name = match.group(1)
            if name not in _KEYWORDS:
                definitions.add(name)
    return definitions


def main(argv: list[str]) -> int:
    root = Path(argv[1] if len(argv) > 1 else ".")
    declarations = collect_declarations(root)
    definitions = collect_definitions(root)

    missing = [(name, headers) for name, headers in declarations.items()
               if name not in definitions]
    if missing:
        for name, headers in sorted(missing, key=lambda item: item[0]):
            for header in headers:
                print(f"ERROR: {header}: public declaration without definition: {name}",
                      file=sys.stderr)
        return 1

    print(f"OK: {len(declarations)} public declarations, "
          f"{len(definitions)} definitions, no missing definitions")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
