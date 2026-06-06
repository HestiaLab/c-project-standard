# C Project Standard

This document defines a pragmatic baseline for small and medium C projects.

## Core Principles

- Start simple and keep seams visible.
- Put public APIs in `include/`; keep implementation details in `src/`.
- Add abstraction only after repeated concrete use.
- Make build, test, and CI reliable from the first commit.
- Keep documentation short, local, and tied to decisions that affect contributors.

## Required Project Shape

```text
include/<project>/   Public headers
src/<project>/       Library implementation
src/main.c           Optional CLI or app entry point
tests/               Focused CTest tests
cmake/               CMake modules
scripts/             Local check wrappers
doc/                 Short documentation
.github/workflows/   CI
```

## CMake Standard

Use a small top-level `CMakeLists.txt` that delegates to modules:

- `cmake/CompilerOptions.cmake` for warnings, sanitizers, and coverage.
- `cmake/Dependencies.cmake` for third-party dependencies.
- `cmake/Sources.cmake` for source lists.
- `cmake/Tests.cmake` for test targets.
- `cmake/Packaging.cmake` for install and packaging rules.

Installable libraries should generate CMake package config files so downstream
projects can use `find_package(<Project> CONFIG REQUIRED)`.

Libraries should support `BUILD_SHARED_LIBS` unless the project has a reason to
ship only static or only shared artifacts. Public headers should expose a small
export macro and be usable from C++ callers with `extern "C"` guards.

Top-level-only conveniences such as examples, tests, and install rules should
default to on for normal repository builds and off when the project is included
as a subproject.

Recommended presets:

- `debug`
- `release`
- `shared`
- `asan`
- `coverage`

## Testing Standard

- Use CTest as the universal test runner.
- Prefer focused test executables per module or subsystem.
- Keep test helpers in `tests/support/`.
- Do not test implementation details unless the detail is a documented internal contract.
- Bug fixes should include a regression test unless the behavior is only observable manually.
- Installable libraries should include a package smoke test that builds a small
  downstream project with `find_package`.

## Header Boundary Standard

Public:

```text
include/<project>/feature.h
```

Internal:

```text
src/<project>/feature_internal.h
```

Rules:

- Public headers should include only what they need.
- Public types should be stable and documented.
- Internal headers may expose implementation state but must stay out of public consumers.

## CI Standard

Every project should have:

- Debug build and tests.
- Release build and tests.
- Shared-library build and tests when `BUILD_SHARED_LIBS` is supported.
- Sanitizer build where supported.
- Static analysis using at least `cppcheck`.
- clang-tidy when a project provides a `.clang-tidy` configuration.
- Coverage preset validation for projects that provide a coverage preset.
- Package smoke test for installable libraries.

## Documentation Standard

Keep local docs short:

- `README.md` for overview and quick start.
- `CONTRIBUTING.md` for workflow and PR expectations.
- `SECURITY.md` for vulnerability handling.
- `doc/build.md` for build details.
- `doc/testing.md` for test layers and expectations.
- `doc/release.md` for release steps.
- `doc/adr/README.md` for architecture decisions.
- `AGENTS.md` for AI-agent instructions.

## Avoid By Default

Do not add these early unless the project already needs them:

- Plugin registries.
- Descriptor manifests.
- Multi-stage command dispatch systems.
- Global service locator objects.
- Generated code.
- Large vendored dependencies.

Use the smallest design that keeps ownership clear.
