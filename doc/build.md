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
- `asan`
- `coverage`

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
