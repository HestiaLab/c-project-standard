# Testing

Use CTest for automated tests.

```sh
ctest --preset debug --output-on-failure
```

## Expectations

- Bug fixes should include regression tests.
- New public APIs should include success and failure cases.
- Tests should assert behavior, not line coverage.
- Prefer focused test programs over broad integration tests for small modules.

## Manual Checks

Document manual checks in PRs when behavior cannot be exercised by CTest.
