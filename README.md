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
scripts/        Local bootstrap and check entry points
docs/           Short project documentation
docs/guides/    Topic guides
.github/        CI workflows and repository templates
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for branch, commit, and PR conventions.

## Quick Start

Required tools:

- CMake 3.21 or newer.
- A C11 compiler from MSVC, GCC, Clang, or AppleClang.
- Ninja for the default cross-platform presets, or another CMake-supported
  generator through local presets.

On Windows, run the Ninja presets from a Developer PowerShell, Developer
Command Prompt, or VS Code CMake kit that initializes MSVC.

For step-by-step installation instructions per platform, see
[docs/guides/environment.md](docs/guides/environment.md).

Recommended editor tools:

- VS Code with the CMake Tools and clangd extensions.
- Ninja, because it generates `compile_commands.json` for clangd.
- Microsoft C/C++ remains usable as a local fallback; see
  `.vscode/c_cpp_properties.example.json`.

The repository includes portable VS Code recommendations, CMake tasks, clangd
defaults, an optional C/C++ fallback example, and Ninja presets. Compiler
installation paths and per-user kits stay in local user configuration.

First-time setup:

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
```

**Linux/macOS**

```sh
./scripts/bootstrap.sh
```

The bootstrap script checks required tools and runs configure so editors can
load `build/ninja-debug/compile_commands.json`.

For a local Visual Studio, Xcode, or Make workflow, copy
`CMakeUserPresets.example.json` to `CMakeUserPresets.json` and edit it for your
machine. The real user preset file is ignored by Git.

Daily check:

**Windows PowerShell**

```powershell
./scripts/check.ps1
```

**Linux/macOS**

```sh
./scripts/check.sh
```

Optional clang-tidy checks:

**Windows PowerShell**

```powershell
./scripts/check.ps1 -EnableTidy
```

**Linux/macOS**

```sh
./scripts/check.sh --enable-tidy
```

For a fuller explanation of configure, build, test, install, presets, and common
errors, see [docs/guides/cmake.md](docs/guides/cmake.md).

## Documentation Map

See [docs/README.md](docs/README.md) for the complete documentation index.

## How To Use This Template

1. Run the rename script to replace all project identifiers in one step:

   **Linux/macOS**
   ```sh
   ./scripts/rename-project.sh MyNewLib
   ```

   **Windows PowerShell**
   ```powershell
   .\scripts\rename-project.ps1 MyNewLib
   ```

   The script renames the project in `CMakeLists.txt`, the public include
   namespace, cmake targets, macro prefixes, config files, and all
   references in source and documentation.

2. Replace the example source and tests with real modules.
3. Keep public APIs in `include/` and private helpers in `src/`.
4. Keep generated version metadata wired to the CMake `project(... VERSION ...)` value.
5. Keep project docs short and update them when build, test, release, or API rules change.

See [docs/guides/template.md](docs/guides/template.md) for the full rename checklist,
including how to trim optional pieces.

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
