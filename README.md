# C Project Standard

> [中文版](README.zh.md)

Start your next C library or CLI tool with a solid foundation — CMake, C11,
CTest, cross-platform CI, and clear contributor docs, all in one template.

## Layout

```text
include/        Public headers
src/            Implementation files
tests/          CTest test programs
cmake/          CMake modules
scripts/        Bootstrap, check, rename, and release scripts
docs/           Project documentation
docs/guides/    Topic guides
.github/        CI workflows and repository templates
```

## Use This Template

1. **Build and run the setup wizard:**

   ```sh
   cmake --preset ninja-debug
   cmake --build --preset ninja-debug
   ./build/ninja-debug/bin/cproject
   ```

   The default `cproject` command starts an interactive wizard that creates a
   new project from this template. It asks for project name, output directory,
   metadata, feature selection, and then initializes the generated copy.

   To skip interaction:

   ```sh
   ./build/ninja-debug/bin/cproject new MyLib --yes
   ```

   Preview without changes:

   ```sh
   ./build/ninja-debug/bin/cproject new MyLib --dry-run --yes
   ```

   To intentionally initialize the current template directory in place, use:

   ```sh
   ./build/ninja-debug/bin/cproject init --in-place MyLib --yes
   ```

2. **Replace** the example source and tests with your real modules.

3. **Keep** public APIs in `include/`, private helpers in `src/`.

4. **Wire** version metadata to the CMake `project(... VERSION ...)` value.

5. **Update** docs when build, test, release, or API rules change.

See [docs/guides/template.md](docs/guides/template.md) for the full rename
checklist, including how to trim optional pieces.

## Quick Start

**Required tools:** CMake 3.21+, a C11 compiler (MSVC, GCC, Clang, or
AppleClang), and Ninja for the default presets.

On Windows, run Ninja presets from a Developer PowerShell, Developer Command
Prompt, or VS Code CMake kit that initializes MSVC. For step-by-step platform
instructions, see [docs/guides/environment.md](docs/guides/environment.md).

**First-time setup:**

Windows PowerShell
```powershell
./scripts/bootstrap.ps1
```

Linux/macOS
```sh
./scripts/bootstrap.sh
```

The bootstrap script checks required tools and runs configure so editors can
load `build/ninja-debug/compile_commands.json`.

**Daily check:**

Windows PowerShell
```powershell
./scripts/check.ps1
```

Linux/macOS
```sh
./scripts/check.sh
```

For a fuller explanation of configure, build, test, install, presets, and
common errors, see [docs/guides/cmake.md](docs/guides/cmake.md).

## What's Included

- CMake modules split by responsibility (compiler options, dependencies,
  sources, tests, packaging).
- C11 as the target-level language baseline.
- Cross-platform support for MSVC, GCC, Clang, and AppleClang.
- Presets for debug, release, shared-library, sanitizer, and coverage builds.
- CTest-based unit tests.
- Installable CMake package config with `find_package` smoke tests.
- `add_subdirectory()` smoke tests for subproject consumers.
- GitHub Actions for build, test, and static analysis.
- PR, issue, and Dependabot templates.
- VS Code recommendations, CMake tasks, and clangd defaults.
- An example library and executable showing the expected layout.

## Package Consumption

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

The `tests/package_smoke/` project validates installed C and C++ consumers.
Direct source-tree consumption through `add_subdirectory()` is validated by
`tests/subproject_smoke/`.

See [C_PROJECT_STANDARD.md](C_PROJECT_STANDARD.md) for the full guidance.

## Documentation

See [docs/README.md](docs/README.md) for the complete documentation index.

## License

MIT
