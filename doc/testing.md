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

## Static Analysis

CI runs clang-format, clang-tidy, and cppcheck. Local clang-tidy checks are
optional because tool availability varies by platform:

```sh
./scripts/check.sh debug tidy
```

```powershell
./scripts/check.ps1 -EnableTidy
```

## Manual Checks

Document manual checks in PRs when behavior cannot be exercised by CTest.
