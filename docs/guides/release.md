# Release

This template provides CPack-based multi-platform packaging and a GitHub
Actions workflow that creates installers for Linux, macOS, and Windows.

## Packaging

The project uses CPack to generate platform-specific installers:

| Platform | Generators           | Artifact                          |
| -------- | -------------------- | --------------------------------- |
| Linux    | DEB, RPM, TGZ        | `.deb`, `.rpm`, `.tar.gz`         |
| macOS    | productbuild, TGZ    | `.pkg`, `.tar.gz`                 |
| Windows  | NSIS, ZIP            | `.exe`, `.zip`                    |

Build the package locally:

```sh
cmake --preset ninja-release
cmake --build --preset ninja-release
cd build/ninja-release && cpack
```

Packages appear in `build/ninja-release/`.

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
2. Runs CPack to generate platform installers.
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

| Path                        | Contents                       |
| --------------------------- | ------------------------------ |
| `bin/`                      | `cproject_example` wizard      |
| `include/`                  | Public C headers               |
| `lib/`                      | `libcproject_core` library     |
| `lib/cmake/CProjectStandard/` | CMake `find_package` configs |
| `share/c-project-standard/` | Full project template sources  |

### After installation

The wizard needs the template files to scaffold new projects.
After installing:

**Linux / macOS**
```sh
cd /usr/local/share/c-project-standard
./cproject_example MyProject --yes
```

**Windows**
```sh
cd "C:\Program Files\CProjectStandard\share\c-project-standard"
cproject_example MyProject --yes
```

Do not publish releases from AI-assisted changes without maintainer approval.
