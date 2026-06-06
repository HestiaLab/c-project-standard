# Testing

Use CTest for automated tests.

```sh
ctest --preset debug --output-on-failure
```

## Expectations

- Bug fixes should include regression tests.
- New public APIs should include success and failure cases.
- Numeric APIs should cover relevant boundary and overflow cases.
- Tests should assert behavior, not line coverage.
- Prefer focused test programs over broad integration tests for small modules.
- Installable libraries should keep a small downstream package smoke test.

## Static Analysis

CI runs clang-format, clang-tidy, and cppcheck. Local clang-tidy checks are
optional because tool availability varies by platform:

```sh
./scripts/check.sh debug tidy
```

```powershell
./scripts/check.ps1 -EnableTidy
```

## Package Smoke

`tests/package_smoke/` is a downstream consumer used to verify installed package
metadata. It should stay small and only assert that installed headers, exported
targets, C linkage, and basic library calls work through `find_package`.

## Manual Checks

Document manual checks in PRs when behavior cannot be exercised by CTest.
