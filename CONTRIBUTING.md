# Contributing

This project uses focused, reviewable changes.

## Local Checks

```sh
./scripts/check.sh
```

On Windows PowerShell:

```powershell
./scripts/check.ps1
```

Or run CMake directly:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

## Branches

Use short branch names:

- `feature/<topic>`
- `fix/<topic>`
- `refactor/<topic>`
- `infra/<topic>`
- `release/<version>`

## Commits And PRs

Use Conventional Commit style where practical:

```text
fix(parser): reject invalid delimiters
feat(cli): add version command
refactor(build): share test target helper
```

Before opening a PR:

- Run the smallest local checks that cover the change.
- Explain behavioral, compatibility, or release impact.
- Add tests for behavior changes.
- Keep unrelated cleanup out of feature or fix PRs.

## Architecture

- Keep public headers small and stable.
- Keep implementation details out of `include/`.
- Prefer explicit ownership and cleanup paths.
- Avoid adding abstractions until repeated use justifies them.
