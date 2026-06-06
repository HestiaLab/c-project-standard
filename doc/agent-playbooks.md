# AI Agent Playbooks

## General Workflow

1. Read `AGENTS.md` and relevant docs.
2. Inspect the smallest relevant file set.
3. Make focused changes.
4. Run relevant checks.
5. Summarize changed files, validation, and residual risk.

For CMake, package, install, or subproject work, use `doc/cmake-guide.md` to
select the relevant smoke checks.

## Bug Fix

1. Reproduce or identify the failing behavior.
2. Add a regression test when practical.
3. Fix the smallest owning module.
4. Run targeted and relevant broader checks.

## Refactor

1. Define behavior that must remain unchanged.
2. Keep changes within module boundaries.
3. Avoid mixing feature behavior with refactor work.
4. Run tests covering touched modules.

## CI Or Build Fix

1. Inspect the failing check and logs.
2. Reproduce locally when practical.
3. Fix the project cause rather than masking the check.
4. Keep scripts and presets aligned.
