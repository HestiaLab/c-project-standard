# Using This Template

Use this checklist when starting a new project from this repository.

## Rename Project Identity

Update these names together:

- `project(CProjectStandard ...)` in `CMakeLists.txt`.
- Public include namespace under `include/cproject/`.
- Implementation namespace under `src/cproject/`.
- CMake target names such as `cproject_core`.
- Package config names in `cmake/Packaging.cmake` and
  `cmake/CProjectStandardConfig.cmake.in`.
- Test target names in `cmake/Tests.cmake`.
- README title, CI workflow names, and documentation examples.

## Replace The Example

- Replace `include/cproject/example.h` with the first real public API.
- Replace `src/cproject/example.c` with the owning implementation.
- Replace `src/main.c` if the project does not ship a CLI or example program.
- Keep tests focused on public behavior in `tests/`.

## Keep Generated Metadata

Version metadata is generated from `project(... VERSION ...)` into
`cproject/version.h`. Keep runtime version reporting wired to that generated
header instead of duplicating literal version numbers in source files.

## Validate The Renamed Project

Run the local checks after renaming:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
cmake --preset shared
cmake --build --preset shared
ctest --preset shared --output-on-failure
```

If the project remains installable, also run a package smoke test using
`tests/package_smoke/` as the downstream consumer pattern.
