# Coding Style

This guide describes the C coding conventions used by this template. It is
deliberately short: the compiler flags and the `.clang-format`/`.clang-tidy`
files are the enforceable baseline, and this document records the intent behind
the choices that tooling cannot check.

## Language

- C11, expressed as the CMake target feature `c_std_11`.
- Keep public code portable across MSVC, GCC, Clang, and AppleClang. Isolate
  compiler-specific constructs behind the macros in
  `include/<project>/compiler.h` instead of scattering `#ifdef` blocks.

## Naming

| Object | Convention | Example |
| --- | --- | --- |
| Files and directories | snake_case | `example.c`, `compiler.h` |
| Functions and variables | snake_case, prefixed with the project namespace | `cproject_add_checked` |
| Types | snake_case + `_t` suffix | `cproject_version_t` |
| Macros and constants | UPPER_SNAKE_CASE | `CPROJECT_API`, `CPROJECT_VERSION_MAJOR` |
| Include guards | `<PROJECT>_<FILE>_H` | `CPROJECT_EXAMPLE_H` |

Function-like macros are written in UPPER_SNAKE_CASE and functions in
snake_case. This distinction is load-bearing: `scripts/check_decl_def.py` uses
it to tell a macro (such as `CPROJECT_DEPRECATED(...)`) from a declared public
function.

## Header Organization

- Public, cross-module interfaces live under `include/<project>/`.
- Give every public header an include guard named `<PROJECT>_<FILE>_H`.
- Keep public headers usable from C++ callers with `extern "C"` guards.
- A public header includes only what it needs.
- Implementation-private headers live next to their source under `src/` and
  never leak into `include/`.

## Formatting and Static Analysis

- `clang-format` (LLVM base, 4-space indent, 100 columns) is the formatting
  authority. The check scripts run `clang-format --dry-run --Werror`.
- `clang-tidy` runs with warnings-as-errors in CI. Run it locally with
  `./scripts/check.ps1 -EnableTidy` (Windows) or
  `./scripts/check.sh --enable-tidy` (Linux/macOS).
- Vendored third-party code under `tests/vendor/` is excluded from
  `clang-format`, `clang-tidy`, and `cppcheck`. Do not hand-format or hand-fix
  vendored code.

## Comments and API Documentation

- Public headers document module, type, and function contracts with Doxygen
  `/** ... */` comments.
- Every public function gets a one-line `@brief`. Non-void functions get a
  `@return` that describes each return value. `@param` entries document NULL
  constraints, valid ranges, borrowed vs owned pointers, and the failure state
  of out parameters.
- Struct and enum fields get a trailing `/**< ... */` comment only when the
  meaning, unit, range, or ownership is not obvious from the name.
- Implementation files do not duplicate the public header's contract comments.
  Local comments explain why, not what.

## Ownership and Error Handling

- Make ownership explicit: the function that allocates is responsible for
  releasing, unless the ownership is transferred and documented in the header.
- Prefer returning an error/status code with out parameters for results, as
  shown by `cproject_add_checked`, over hidden global state.

## File Encoding

Source files are UTF-8, with LF line endings (enforced by `.gitattributes`).
