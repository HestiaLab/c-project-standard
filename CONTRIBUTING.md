# Contributing

This project uses focused, reviewable changes.

## Local Checks

Before submitting, run the complete local check (format check + configure +
build + test). The check commands do not rewrite source automatically; a format
failure fails the check.

**Linux/macOS**

```sh
./scripts/bootstrap.sh
./scripts/check.sh
```

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
./scripts/check.ps1
```

Run the full quality gate including clang-tidy before submitting:

**Linux/macOS**

```sh
./scripts/check.sh --enable-tidy
```

**Windows PowerShell**

```powershell
./scripts/check.ps1 -EnableTidy
```

To fix C formatting in place explicitly, run:

```sh
./scripts/check.sh --fix-format        # Linux/macOS
./scripts/check.ps1 -FixFormat         # Windows
```

To check or format C files on their own, without configure/build/test, use the
dedicated format script:

```powershell
./scripts/format.ps1 -Check             # verify only; exits 2 on violations
./scripts/format.ps1 -Format            # rewrite in place
```

`-SkipFormat` / `--skip-format` is for temporary diagnosis only and does not
satisfy the pre-submit check. CI re-runs the full quality gate without skip
flags. `clang-format` does not process Markdown; for documentation changes,
review the prose, relative links, and `git diff --check` output instead.
Vendored third-party code under `tests/vendor/` is excluded from format and
static-analysis checks.

For a new checkout or a new machine, run bootstrap first. If you have not yet
installed the required tools, see
[docs/en/guides/environment.md](docs/en/guides/environment.md) for the Windows
development environment setup.

For a detailed explanation of configure, build, test, install, presets, and
common CMake troubleshooting, see [docs/en/guides/cmake.md](docs/en/guides/cmake.md).

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
- Fill in the PR template sections that apply to the change.
- Explain behavioral, compatibility, or release impact.
- Add tests for behavior changes.
- Link to updated docs when the change affects build, test, install, release,
  security, or public API behavior.
- Keep unrelated cleanup out of feature or fix PRs.

Use the issue templates for bug reports, feature proposals, and infrastructure
maintenance requests.

## Architecture

When modifying core documentation (build, testing, template), also update the
corresponding Chinese translation under `docs/zh/` if the change affects
user-visible instructions or policy. English guides live under `docs/en/`;
Chinese mirrors live under `docs/zh/`.

- Keep public headers small and stable.
- Keep implementation details out of `include/`.
- Prefer explicit ownership and cleanup paths.
- Avoid adding abstractions until repeated use justifies them.
