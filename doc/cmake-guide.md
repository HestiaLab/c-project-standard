# CMake Guide

This guide explains the common CMake workflows in this template. Use it when
you are new to CMake or when you need to add source files, tests, install rules,
or downstream consumers.

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
