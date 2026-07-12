# Release

This template provides multi-platform packaging and a GitHub Actions
workflow that creates installers for Linux, macOS, and Windows.

## Packaging

Platform-specific installers are built with CPack (Linux/macOS) and WiX (Windows):

| Platform | Generators                    | Artifact                   |
| -------- | ----------------------------- | -------------------------- |
| Linux    | DEB, RPM, TGZ                 | `.deb`, `.rpm`, `.tar.gz`  |
| macOS    | productbuild, TGZ             | `.pkg`, `.tar.gz`          |
| Windows  | WiX (MSI), ZIP (CPack)        | `.msi`, `.zip`             |

Build the package locally:

```sh
cmake --preset ninja-release
cmake --build --preset ninja-release

# Linux / macOS
cd build/ninja-release && cpack

# Windows
cmake --build --preset ninja-release --target msi
```

Packages appear in `build/ninja-release/` (Linux/macOS) or
`build/ninja-release/msi/` (Windows).

### Windows MSI prerequisites

The Windows MSI requires the WiX Toolset v6 and two extensions. Install them
once:

```sh
dotnet tool install --global wix --version 6.*
wix extension add WixToolset.UI.wixext
wix extension add WixToolset.Util.wixext
```

### Source packages

```sh
cmake --preset ninja-debug
cd build/ninja-debug && cpack --config CPackSourceConfig.cmake
```

## Release workflow

The `.github/workflows/release.yml` workflow:

1. **Automatic**: pushes a tag matching `v*` (e.g. `v0.2.0`).
2. **Manual**: `workflow_dispatch` from the Actions tab.

The workflow:

1. Builds `ninja-release` on `ubuntu-latest`, `macos-latest`, `windows-latest`.
2. Runs CPack (Linux/macOS) or CMake `msi` target (Windows) to generate platform installers.
3. Builds source archives (`tar.gz` + `zip`).
4. On tag push, creates a GitHub Release with all artifacts attached.

## Release checklist

1. Update version in `CMakeLists.txt` (`project(... VERSION x.y.z)`).
2. Update `CHANGELOG.md`.
3. Run local verification:

   **Windows PowerShell**
   ```powershell
   ./scripts/check.ps1 -Preset ninja-release
   cmake --preset ninja-shared
   cmake --build --preset ninja-shared
   ctest --preset ninja-shared --output-on-failure
   ```

   **Linux/macOS**
   ```sh
   ./scripts/check.sh --preset ninja-release
   cmake --preset ninja-shared
   cmake --build --preset ninja-shared
   ctest --preset ninja-shared --output-on-failure
   ```

4. Run static and shared package smoke tests (see `cmake.md`).
5. Commit and push.
6. Tag the release:

   ```sh
   git tag -a v0.2.0 -m "v0.2.0"
   git push origin v0.2.0
   ```

7. Monitor the Release workflow in GitHub Actions.
8. Verify the generated release and installers.

## Package contents

Each binary installer installs:

| Path                          | Contents                       |
| ----------------------------- | ------------------------------ |
| `bin/`                        | `cproject` project wizard      |
| `include/`                    | Public C headers               |
| `lib/`                        | `libcproject_core` library     |
| `lib/cmake/CProjectStandard/` | CMake `find_package` configs   |
| `share/c-project-standard/`   | Full project template sources  |

### After installation

The installed `cproject` command uses `share/c-project-standard/` as a read-only
template source and creates projects in a user-writable output directory. After
installing, open a new terminal and run:

```sh
cproject
```

or create a project non-interactively:

```sh
cproject new MyProject --yes
```

On Windows, the installer also creates Start Menu and desktop shortcuts for the
Project Setup Wizard.

### Windows installer manual checks

Before publishing a Windows release, install the generated `.msi` and verify:

1. The installer wizard pages render correctly (license, install directory, progress).
2. A desktop shortcut named `CProjectStandard` exists on the **current user's personal desktop**.
3. The Start Menu folder contains `Project Setup Wizard`, `Template Files`, and `Uninstall`.
4. A new terminal can run `cproject --version` and `cproject new MyProject --yes`.
5. The generated project configures, builds, and passes tests.
6. Uninstall (via *Apps & Features* or `msiexec /x`) removes the desktop shortcut and Start Menu entries.

Do not publish releases from AI-assisted changes without maintainer approval.