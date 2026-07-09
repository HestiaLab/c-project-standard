# AGENTS.md

Repository-specific instructions for AI agents working on this template and
projects scaffolded from it.

## Required Reading

Before changing files, read:

- `CONTRIBUTING.md`
- `docs/guides/testing.md`

For architecture-sensitive changes, also read:

- `docs/adr/README.md`
- Relevant ADR files under `docs/adr/`

For build, CI, packaging, release, dependency, or security work, also read:

- `docs/guides/cmake.md`
- `docs/guides/release.md`
- `SECURITY.md`

## Project Shape

This is a C11 project template using CMake and CTest.

```text
include/        Public headers
src/            Implementation
tests/          CTest tests
cmake/          Build modules
scripts/        Local bootstrap and check wrappers
docs/           Contributor documentation
docs/guides/    Topic guides
```

## Build And Verification

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

The build produces `build/ninja-debug/bin/cproject_example` — this is the
**interactive project setup wizard**. Run it to initialize a new project from
this template. Use `--dry-run --yes` to preview, or `MyProject --yes` for
non-interactive setup.

On Windows PowerShell:

```powershell
./scripts/check.ps1
```

On Linux/macOS:

```sh
./scripts/check.sh
```

## Ownership Rules

- Public APIs belong in `include/`.
- Private implementation helpers belong in `src/`.
- Tests should cover behavior, not accidental implementation details.
- Keep CMake target dependencies narrow.
- Do not add new global state without a documented reason.
- Do not weaken warnings, tests, or CI to make a change pass.

## AI Agent Policy

AI agents may help with development when their changes are scoped, reviewed,
and verified like any other contribution.

### Appropriate Uses

- Focused bug fixes.
- Test additions.
- Documentation updates.
- Build and CI maintenance.
- Mechanical refactors that preserve behavior.

### Restricted Uses

AI agents should not independently:

- Redesign core architecture without maintainer direction.
- Weaken tests, warnings, static analysis, or CI.
- Add large dependencies or generated code without approval.
- Publish releases, rotate credentials, or change repository permissions.

### Disclosure

Pull requests should disclose meaningful AI assistance: list the required docs
read, summarize what the agent did, and record the human verification performed.

### Playbooks

**General workflow:** Read AGENTS.md and relevant docs, inspect the smallest
relevant file set, make focused changes, run checks, summarize.

**Bug fix:** Reproduce or identify the failing behaviour, add a regression test
when practical, fix the smallest owning module, run targeted and broader checks.

**Refactor:** Define behaviour that must remain unchanged, stay within module
boundaries, avoid mixing feature behaviour with refactor work, run tests
covering touched modules.

**CI or build fix:** Inspect the failing check and logs, reproduce locally when
practical, fix the project cause rather than masking the check, keep scripts
and presets aligned.

For CMake, package, install, or subproject work, use `docs/guides/cmake.md`
to select the relevant smoke checks.

## Documentation

Update local docs when build commands, CMake usage, dependencies, testing policy,
release policy, security policy, or public API expectations change.
