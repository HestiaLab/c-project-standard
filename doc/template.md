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

## Trim Optional Pieces

Keep the template small after renaming. Remove pieces that do not match the new
project:

- Remove `src/main.c` and `CPROJECT_BUILD_EXAMPLE` if the project has no CLI or
  runnable example.
- Remove `tests/package_smoke/` and package install docs if the project is not
  intended to be installed or consumed through CMake package config.
- Remove `tests/subproject_smoke/` if the project will never be consumed with
  `add_subdirectory()`.
- Remove the `coverage` preset and CI job if coverage data will not be used.
- Remove AI-agent docs if the downstream project does not accept AI-assisted
  changes.
- Keep `.gitattributes`, formatting rules, and local check scripts unless the
  downstream project has a stronger existing policy.
