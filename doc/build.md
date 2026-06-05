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
