# C Project Standard

A compact C11 project template and engineering standard for new C projects.

This repository captures reusable infrastructure patterns:

- CMake modules split by responsibility.
- C11 as the target-level language baseline.
- Cross-platform compiler support for MSVC, GCC, Clang, and AppleClang.
- CMake presets for debug, release, sanitizer, and coverage builds.
- Static and shared library builds through `BUILD_SHARED_LIBS`.
- CTest based unit tests.
- Installable CMake package config with downstream smoke tests.
- `add_subdirectory()` smoke tests for subproject consumers.
- GitHub Actions for build, test, and static analysis.
- GitHub PR, issue, and Dependabot templates for repository maintenance.
- Short contributor, testing, release, security, and AI-agent guidance.
- A small example library and executable showing the expected layout.

## Layout

```text
include/        Public headers
src/            Implementation files
tests/          CTest test programs
cmake/          CMake modules
scripts/        Local check entry points
doc/            Short project documentation
doc/guides/     Topic guides
.github/        CI workflows and repository templates
```

## Quick Start

Required tools:

- CMake 3.20 or newer.
- A C11 compiler from MSVC, GCC, Clang, or AppleClang.
- A CMake-supported generator such as Visual Studio, Xcode, Ninja, or Make.

Recommended editor tools:

- VS Code with the CMake Tools extension.
- Microsoft C/C++ extension or clangd for code navigation.
- Ninja when using clangd, because it can generate `compile_commands.json`.

The repository includes portable VS Code recommendations, CMake tasks, C/C++
fallback include paths, and a small `.clangd` fallback. Compiler installation
paths and per-user kits stay in local user configuration.

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

On Windows PowerShell:

```powershell
./scripts/check.ps1
```

On Linux/macOS:

```sh
./scripts/check.sh
```

Optional clang-tidy checks:

```powershell
./scripts/check.ps1 -EnableTidy
```

```sh
./scripts/check.sh debug tidy
```

For a fuller explanation of configure, build, test, install, presets, and common
errors, see [doc/guides/cmake.md](doc/guides/cmake.md).

## Documentation Map

- [doc/README.md](doc/README.md) maps each topic to its source of truth.
- [doc/guides/cmake.md](doc/guides/cmake.md) explains day-to-day CMake usage.
- [doc/guides/testing.md](doc/guides/testing.md) describes test and static-analysis
  expectations.
- [doc/guides/template.md](doc/guides/template.md) explains how to rename and
  trim this template.
- [CONTRIBUTING.md](CONTRIBUTING.md) describes branch, commit, and PR workflow.
- [SECURITY.md](SECURITY.md) describes vulnerability and dependency handling.
- [C_PROJECT_STANDARD.md](C_PROJECT_STANDARD.md) defines the reusable project standard.

## How To Use This Template

1. Rename the project in `CMakeLists.txt`.
2. Rename the public include namespace under `include/cproject/`.
3. Replace the example source and tests with real modules.
4. Keep public APIs in `include/` and private helpers in `src/`.
5. Keep generated version metadata wired to the CMake `project(... VERSION ...)` value.
6. Keep project docs short and update them when build, test, release, or API rules change.

See [doc/guides/template.md](doc/guides/template.md) for a rename checklist.

## Installed Package Use

The install rules generate a CMake package config:

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

The `tests/package_smoke/` project validates installed C and C++ consumers.

Direct source-tree consumption through `add_subdirectory()` is validated by
`tests/subproject_smoke/`.

See [C_PROJECT_STANDARD.md](C_PROJECT_STANDARD.md) for the full guidance.
