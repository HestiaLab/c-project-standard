# Build

## Requirements

- CMake 3.20 or newer
- Ninja, Make, or an IDE generator supported by CMake
- A C11 compiler

## Presets

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Available presets:

- `debug`
- `release`
- `shared`
- `asan`
- `coverage`

Common options:

- `BUILD_SHARED_LIBS` controls static or shared library output.
- `CPROJECT_BUILD_EXAMPLE` builds the example executable.
- `CPROJECT_BUILD_TESTING` builds CTest test targets.
- `CPROJECT_INSTALL` generates install rules.

The example, tests, and install rules default to on for top-level builds and
off when this project is included as a subproject.

## Local Check Scripts

```sh
./scripts/check.sh
```

```powershell
./scripts/check.ps1
```

Optional clang-tidy checks require `clang-tidy`:

```sh
./scripts/check.sh debug tidy
```

```powershell
./scripts/check.ps1 -EnableTidy
```

## Install And Consume

Install a configured build with CMake:

```sh
cmake --install build/release --prefix install
```

Installed consumers can use the generated package config:

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

Project version metadata is generated from the top-level
`project(... VERSION ...)` value into `cproject/version.h`.

The package smoke test under `tests/package_smoke/` demonstrates downstream
usage for an installed package:

```sh
cmake --install build/release --prefix install
cmake -S tests/package_smoke -B build/package-smoke -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

The shared install path should also remain consumable:

```sh
cmake --preset shared
cmake --build --preset shared
cmake --install build/shared --config Debug --prefix install-shared
cmake -S tests/package_smoke -B build/package-smoke-shared -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/install-shared"
cmake --build build/package-smoke-shared --config Debug
ctest --test-dir build/package-smoke-shared -C Debug --output-on-failure
```

`tests/subproject_smoke/` verifies `add_subdirectory()` usage:

```sh
cmake -S tests/subproject_smoke -B build/subproject-smoke -DCPROJECT_SOURCE_DIR="$PWD"
cmake --build build/subproject-smoke --config Debug
ctest --test-dir build/subproject-smoke -C Debug --output-on-failure
```
