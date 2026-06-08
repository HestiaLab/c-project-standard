# CMake Guide

This guide explains the common CMake workflows in this template. Use it when
you are new to CMake or when you need to add source files, tests, install rules,
or downstream consumers.

## Requirements

These tools are not provided by the repository. Install them before configuring
the project.

- CMake 3.20 or newer.
- A C11 compiler.
- One supported compiler family:
  - MSVC on Windows.
  - GCC or Clang on Linux.
  - AppleClang or Clang on macOS.
- Ninja, Make, Visual Studio, Xcode, or another generator supported by CMake.

The project requires C11 at the target level with `c_std_11`. Keep public
headers portable across the supported compiler families and avoid compiler
extensions unless they are isolated behind small CMake/compiler checks.

## Tool Installation Notes

Typical setup choices:

| Platform | Compiler | Generator |
| --- | --- | --- |
| Windows | MSVC from Visual Studio Build Tools or Visual Studio | Visual Studio or Ninja |
| Linux | GCC or Clang from the system package manager | Ninja or Make |
| macOS | AppleClang from Xcode Command Line Tools or Clang | Xcode, Ninja, or Make |

VS Code is optional. If you use it, install CMake Tools plus one C/C++ language
service. Use Microsoft C/C++ for Visual Studio-style CMake integration, or use
clangd with a generator that produces `compile_commands.json`, such as Ninja or
Unix Makefiles. Do not enable multiple C/C++ language services for the same
workspace unless you intentionally manage their overlap.

This template commits portable editor defaults:

- `.vscode/extensions.json` recommends CMake Tools and Microsoft C/C++.
- `.vscode/settings.json` tells VS Code to use CMake presets and CMake Tools as
  the C/C++ configuration provider.
- `.vscode/c_cpp_properties.json` provides platform-named fallback include paths
  without hard-coded compiler installation paths.
- `.vscode/tasks.json` provides configure, build, and test tasks that call CMake
  and CTest directly.
- `.clangd` provides fallback project include paths for clangd users.

Keep these files generic. Do not commit absolute compiler paths such as a local
Visual Studio, MSYS2, Homebrew, or system SDK installation. Put those paths in
VS Code user settings, CMake kits, `CMakeUserPresets.json`, shell environment
setup, or a local toolchain file.

## Mental Model

CMake work usually has four separate steps:

1. Configure: read `CMakeLists.txt`, choose options, and generate build files.
2. Build: compile sources and link targets using the generated build files.
3. Test: run CTest tests from the configured build tree.
4. Install: copy public artifacts into an install prefix for downstream use.

Those steps are intentionally separate. If configure fails, the build files are
not reliable. If build fails, tests and install are not meaningful.

## Quick Commands

Use the debug preset for normal development:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

On Windows PowerShell, the local wrapper runs the same flow:

```powershell
./scripts/check.ps1
```

On Linux or macOS:

```sh
./scripts/check.sh
```

## Presets

Presets are named build configurations stored in `CMakePresets.json`.

| Preset | Purpose |
| --- | --- |
| `debug` | Fast local development with debug symbols. |
| `release` | Optimized build for release-style validation and install smoke tests. |
| `shared` | `debug` build with `BUILD_SHARED_LIBS=ON`. |
| `asan` | Debug build with AddressSanitizer and UBSan where supported. |
| `coverage` | Debug build with coverage flags where supported. |

Configure, build, and test presets share the same names:

```sh
cmake --preset shared
cmake --build --preset shared
ctest --preset shared --output-on-failure
```

Presets intentionally do not force a compiler or generator. That keeps the same
repository usable with Visual Studio on Windows, Make or Ninja on Linux, and
Xcode or Ninja on macOS. Select the compiler and generator with your CMake kit,
environment, or command-line `-G`/toolchain options before configuring a build
directory.

For editor language servers that need `compile_commands.json`, prefer a
single-config generator such as Ninja or Unix Makefiles:

```sh
cmake --preset debug -G Ninja
```

Visual Studio and Xcode builds remain supported, but they are multi-config
generators. Use CMake Tools as the configuration provider for IDE IntelliSense
or add editor-local include paths for generated headers when the language server
cannot consume the generator metadata directly.

All shared presets inherit a base preset that enables
`CMAKE_EXPORT_COMPILE_COMMANDS`. Generators that support compilation databases
write `compile_commands.json` into the preset build directory.

## Important Options

These options can be passed during configure:

```sh
cmake --preset debug -DCPROJECT_BUILD_EXAMPLE=OFF
```

| Option | Meaning |
| --- | --- |
| `BUILD_SHARED_LIBS` | Builds libraries as shared libraries when supported. |
| `CPROJECT_BUILD_EXAMPLE` | Builds `src/main.c` as the example executable. |
| `CPROJECT_BUILD_TESTING` | Builds CTest test targets. |
| `CPROJECT_INSTALL` | Generates install rules. |
| `CPROJECT_ENABLE_ASAN` | Enables AddressSanitizer where supported. |
| `CPROJECT_ENABLE_UBSAN` | Enables UndefinedBehaviorSanitizer where supported. |
| `CPROJECT_ENABLE_COVERAGE` | Enables coverage flags where supported. |

Examples, tests, and install rules default to on for top-level builds. They
default to off when this project is included through `add_subdirectory()`.

Use `cmake -LAH -S . -B build/debug` after configuring if you need to inspect
available cache options.

## Multi-Config Generators

Visual Studio and Xcode are multi-config generators. They can produce Debug and
Release artifacts from the same build directory, so build and test commands need
an explicit config in some workflows:

```sh
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

Single-config generators such as Ninja and Make use `CMAKE_BUILD_TYPE` at
configure time instead.

## Toolchain Policy

The build definition is the source of truth for language mode and include
directories:

- CMake minimum version: 3.20.
- C language standard: C11, expressed as target feature `c_std_11`.
- Supported compiler families: MSVC, GCC, Clang, and AppleClang.
- Supported generators: any CMake generator that works with the chosen compiler,
  with Visual Studio, Xcode, Ninja, and Make as the normal development paths.

The repository declares requirements and recommended editor extensions, but it
does not install compilers, SDKs, CMake, Ninja, VS Code extensions, or clangd for
users. Keep install commands in contributor docs or platform onboarding notes
when a project needs stricter setup guidance.

Do not put machine-specific compiler paths, SDK paths, or Visual Studio
installation paths in committed presets. Use user presets, CMake kits, a
toolchain file, or environment setup scripts for local toolchain selection.

Keep generated headers under the build tree and expose them through target
include directories. Editors should learn those paths from CMake metadata or
from local workspace configuration; source files should not include generated
headers by relative filesystem paths.

## Add A Source File

Add implementation files under `src/cproject/` and public headers under
`include/cproject/`.

1. Create the `.c` file under `src/cproject/`.
2. Create or update the matching public header under `include/cproject/`.
3. Add the new `.c` file to `CPROJECT_CORE_SOURCES` in `cmake/Sources.cmake`.
4. Add tests under `tests/`.
5. Run `./scripts/check.ps1` or `./scripts/check.sh`.

Public headers should include only what they need and should remain usable from
C++ callers when they expose C APIs.

## Add A Test

Use `cproject_add_test()` from `cmake/Tests.cmake`:

```cmake
cproject_add_test(cproject_feature_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_feature.c
    LIBS
        cproject_core
)
```

Then run:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Tests should assert public behavior. Keep implementation-detail tests rare and
document the internal contract they protect.

## Install And Use With find_package

Configure and build a release package:

```sh
cmake --preset release
cmake --build --preset release
cmake --install build/release --config Release --prefix install
```

A downstream CMake project can then consume it:

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

The package smoke project validates this path:

```sh
cmake -S tests/package_smoke -B build/package-smoke -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

For a shared-library install, use the `shared` preset and a separate install
prefix:

```sh
cmake --preset shared
cmake --build --preset shared
cmake --install build/shared --config Debug --prefix install-shared
cmake -S tests/package_smoke -B build/package-smoke-shared -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/install-shared"
cmake --build build/package-smoke-shared --config Debug
ctest --test-dir build/package-smoke-shared -C Debug --output-on-failure
```

On Windows, shared-library consumers need the installed DLL next to the test
executable or on `PATH`. The smoke project copies the imported runtime DLL when
CMake exposes it through the installed target metadata.

## Use As A Subproject

Projects can also consume this repository directly:

```cmake
add_subdirectory(path/to/c-project-standard)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

The subproject smoke test validates this mode:

```sh
cmake -S tests/subproject_smoke -B build/subproject-smoke -DCPROJECT_SOURCE_DIR="$PWD"
cmake --build build/subproject-smoke --config Debug
ctest --test-dir build/subproject-smoke -C Debug --output-on-failure
```

When included as a subproject, examples, tests, and install rules default to
off. A downstream project can enable them explicitly if needed.

## Generated Version Header

The project version is declared once:

```cmake
project(CProjectStandard VERSION 0.1.0 LANGUAGES C)
```

CMake generates `cproject/version.h` from that value. Source files should use the
generated macros instead of duplicating literal version numbers.

## Local Quality Checks

The check scripts run configure, build, and tests. They also run optional tools
when available:

```powershell
./scripts/check.ps1 -EnableTidy
```

```sh
./scripts/check.sh debug tidy
```

If `clang-format`, `clang-tidy`, or `yamllint` is not installed locally, use CI
as the authoritative check for those tools.

## Troubleshooting

Old CMake cache points at another path:

- Symptom: configure mentions a different source directory.
- Fix: delete the affected build directory, such as `build/release`, and run
  `cmake --preset release` again.

`find_package(CProjectStandard)` cannot find the package:

- Symptom: CMake cannot find `CProjectStandardConfig.cmake`.
- Fix: install the project first and pass the absolute install prefix with
  `-DCMAKE_PREFIX_PATH=/path/to/install`.

Windows shared-library test cannot find the DLL:

- Symptom: an installed shared-library consumer builds but fails at runtime.
- Fix: copy the installed DLL next to the executable or add the install `bin`
  directory to `PATH`.

`ctest` cannot find a test executable with Visual Studio:

- Symptom: CTest looks in the wrong configuration directory.
- Fix: pass `-C Debug` or `-C Release`, matching the build config.

`clang-format`, `clang-tidy`, or `yamllint` is missing:

- Symptom: local scripts skip a tool or the shell cannot find it.
- Fix: install the tool locally, or rely on CI where the workflow installs the
  required toolchain.
