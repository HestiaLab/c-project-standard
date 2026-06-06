# C Project Standard

A compact C11 project template and engineering standard for new C projects.

This repository captures reusable infrastructure patterns:

- CMake modules split by responsibility.
- CMake presets for debug, release, sanitizer, and coverage builds.
- CTest based unit tests.
- GitHub Actions for build, test, and static analysis.
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
.github/        CI workflows
```

## Quick Start

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

## How To Use This Template

1. Rename the project in `CMakeLists.txt`.
2. Rename the public include namespace under `include/cproject/`.
3. Replace the example source and tests with real modules.
4. Keep public APIs in `include/` and private helpers in `src/`.
5. Keep generated version metadata wired to the CMake `project(... VERSION ...)` value.
6. Keep project docs short and update them when build, test, release, or API rules change.

## Installed Package Use

The install rules generate a CMake package config:

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

See [C_PROJECT_STANDARD.md](C_PROJECT_STANDARD.md) for the full guidance.
