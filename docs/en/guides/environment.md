# Environment Setup

This project supports one development environment on Windows 10/11 (64-bit):

> **VS Code + MSYS2 UCRT64 (GCC, GDB, Ninja, clangd) + native CMake + Git**

All build tools come from MSYS2 UCRT64 so there is a single source for GCC, GDB,
Ninja, and clangd. CMake is the native Windows build; Git is Git for Windows.

## Requirements

| Tool | Required | Purpose |
| --- | --- | --- |
| CMake 3.21+ (native Windows) | Yes | Configure and generate build files |
| GCC (MSYS2 UCRT64) | Yes | Compile C11 source |
| Ninja | Yes (default presets) | Build generator for the `ninja-*` presets |
| GDB | Yes (debugging) | Breakpoints, stepping, variables |
| clangd | No | Editor code navigation and diagnostics |
| clang-format | No | Local formatting checks |
| clang-tidy | No | Optional local static analysis |

## Install steps

Install in this order. Keep install paths free of spaces and non-ASCII
characters where practical.

### 1. Visual Studio Code

Install [VS Code](https://code.visualstudio.com/) (64-bit). During install,
enable **Add to PATH** and the *Open with Code* context-menu entries.

```powershell
code --version
```

### 2. Git

Install [Git for Windows](https://git-scm.com/download/win). In the *Adjusting
your PATH environment* step choose **Git from the command line and also from
3rd-party software** so VS Code, CMake, and the MSYS2 shell can all call `git`.

```powershell
git --version
```

### 3. MSYS2

Install [MSYS2](https://www.msys2.org/) to the default path `C:\msys64`. Launch
**MSYS2 UCRT64** from the Start Menu and update it fully:

```sh
pacman -Syu
```

Re-open the UCRT64 shell if the update asks you to, and repeat `pacman -Syu`
until no packages remain. MSYS2 is a rolling release: always update fully before
installing anything, and never use `pacman -Sy` alone to pull a single package.

### 4. GCC, GDB, Ninja, clangd

In the UCRT64 shell, install the formal toolchain:

```sh
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-gdb \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-clang-tools-extra
```

`clang-tools-extra` provides `clangd`, `clang-format`, and `clang-tidy`
(`/ucrt64/bin/clangd.exe`), so no separate LLVM download is needed.

### 5. CMake (native Windows)

Install the **native Windows CMake** (`.msi` from
[cmake.org](https://cmake.org/download/)), not the MSYS2 `/usr/bin` CMake.
Select **Add CMake to the system PATH** during install. Version must be 3.21 or
newer.

```powershell
cmake --version
```

### 6. Windows PATH

Add `C:\msys64\ucrt64\bin` to the user or system PATH. Do **not** place
`C:\msys64\clang64\bin` ahead of it globally; the sanitizer toolchain is outside
the supported environment (see below).

### 7. VS Code extensions

Install these extensions:

```text
ms-vscode.cmake-tools                  # CMake configure/build/debug
llvm-vs-code-extensions.vscode-clangd  # completion, diagnostics, go-to-definition
ms-vscode.cpptools                     # GDB debug adapter (cppdbg)
```

## Sanitizers (ASan/UBSan)

Sanitizers are **not part of the supported environment**. The UCRT64 GCC
distribution does not ship a linkable `libasan`/`libubsan`, and reinstalling
UCRT64 GCC does not add them. AddressSanitizer/UBSan requires the separate MSYS2
CLANG64 toolchain (Clang + compiler-rt), so the `ninja-asan` preset will not work
under UCRT64 GCC alone. Leave sanitizer runs to CI on Linux/macOS, or set up
CLANG64 separately if you need them locally.

## Verification

From the repository root, run the bootstrap checks:

```powershell
./scripts/bootstrap.ps1 -CheckOnly    # tools only
./scripts/bootstrap.ps1 -CheckEditor   # also check gdb
./scripts/bootstrap.ps1                # tools + configure ninja-debug
```

Then confirm each tool resolves to the UCRT64 install rather than a duplicate:

```powershell
where gcc
where gdb
where ninja
where clangd
where cmake
where git
```

`gcc` should resolve to `C:\msys64\ucrt64\bin\gcc.exe` and `gdb` to
`C:\msys64\ucrt64\bin\gdb.exe`. If `where gcc` lists another MinGW first, reorder
PATH so `C:\msys64\ucrt64\bin` comes first.

## Next steps

- **Build and test**: see [cmake.md](cmake.md) for configure, build, test,
  presets, and troubleshooting.
- **Test expectations**: see [testing.md](testing.md) for test layers and
  static analysis.
- **Contributing**: see [../../../CONTRIBUTING.md](../../../CONTRIBUTING.md) for
  branch, commit, and PR workflow.
- **Project standard**: see [../../../C_PROJECT_STANDARD.md](../../../C_PROJECT_STANDARD.md)
  for the full engineering standard.
