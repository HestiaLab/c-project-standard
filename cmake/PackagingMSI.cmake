# ---------------------------------------------------------------------------
# Windows MSI installer — WiX 6 build integration.
#
# Included from CMakeLists.txt only when:
#   * CPROJECT_INSTALL is ON, and
#   * CMAKE_SYSTEM_NAME STREQUAL "Windows".
#
# Build flow (driven by `cmake --build . --target msi`):
#
#   1. install-staging/   populated by `cmake --install --prefix`.
#                         Uses the existing install(DIRECTORY ...) rules in
#                         cmake/InstallRules.cmake, so the MSI ships exactly
#                         the files that `make install` ships.
#
#   2. installer/generated/TemplateTree.wxs   generated at build time from
#                         the staged template tree by
#                         cmake/GenerateWiXTemplateTree.cmake. WiX 6 removed
#                         the v3 `heat` command, so we generate the fragment
#                         ourselves.
#
#   3. msi/   output directory for the built
#             CProjectStandard-<version>-win64.msi.
#
# Why hand-listed Component GUIDs for bin/lib/include but a harvested
# ComponentGroup for the template tree:
#   * bin/lib/include layout is small and stable - explicit GUIDs keep
#     Windows Installer upgrade chains intact.
#   * The template tree changes with every commit - hand-listing it would
#     rot the moment a file is added. Per-file GUIDs derived from SHA-512 of
#     the path keep the harvest output stable.
# ---------------------------------------------------------------------------

include_guard(GLOBAL)

# Install rules are shared with the Linux/macOS packaging path. Including
# them here makes `cmake --install` populate the staging directory used by
# the WiX build.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallRules.cmake")

# ---- Tool detection -------------------------------------------------------

# WiX 6 CLI is installed as a .NET global tool under ~/.dotnet/tools.
find_program(WIX_EXECUTABLE
    NAMES wix
    PATHS "$ENV{USERPROFILE}/.dotnet/tools"
    DOC   "WiX 6 CLI (.NET tool)")

if(NOT WIX_EXECUTABLE)
    message(WARNING
        "WiX 6 was not found — the 'msi' target will not be available. "
        "Install WiX to build the MSI installer:\n"
        "  dotnet tool install --global wix --version 6.*\n"
        "Then re-run cmake.")
    set(CPROJECT_WIX_AVAILABLE OFF)
else()
    set(CPROJECT_WIX_AVAILABLE ON)
endif()

if(CPROJECT_WIX_AVAILABLE)

# WiX 6 stores extensions under:
#   * $LOCALAPPDATA/wix/extensions/ when installed with `wix extension add -g`
#   * <cwd>/.wix/extensions/ when installed without -g (project-local)
# We probe both. Picking the highest installed version lets 6.0.x and 6.1.x
# coexist if a developer happens to have both.
set(_wix_search_dirs
    "$ENV{LOCALAPPDATA}/wix/extensions"
    "$ENV{USERPROFILE}/.wix/extensions"
    "${CMAKE_CURRENT_SOURCE_DIR}/.wix/extensions")
set(_wix_ui_ext_path "")
foreach(_d IN LISTS _wix_search_dirs)
    if(_wix_ui_ext_path)
        break()
    endif()
    if(NOT EXISTS "${_d}")
        continue()
    endif()
    file(GLOB _candidates
            "${_d}/WixToolset.UI.wixext/*/wixext6/WixToolset.UI.wixext.dll")
        if(_candidates)
            # Use the lexicographically last path - good enough to pick the
            # highest version when multiple are installed side by side.
            list(SORT _candidates)
            list(GET _candidates -1 _wix_ui_ext_path)
        endif()
endforeach()

if(NOT _wix_ui_ext_path)
    message(WARNING
        "WixToolset.UI.wixext not found — WixUI dialogs unavailable. "
        "Install it with:\n"
        "  wix extension add WixToolset.UI.wixext\n"
        "Then re-run cmake.")
    set(CPROJECT_WIX_AVAILABLE OFF)
endif()

# Same detection for the Util extension (provides WixQuietExec64 used by the
# desktop-shortcut custom action in Shortcuts.wxs).
set(_wix_util_ext_path "")
foreach(_d IN LISTS _wix_search_dirs)
    if(_wix_util_ext_path)
        break()
    endif()
    if(NOT EXISTS "${_d}")
        continue()
    endif()
    file(GLOB _candidates
        "${_d}/WixToolset.Util.wixext/*/wixext6/WixToolset.Util.wixext.dll")
    if(_candidates)
        list(SORT _candidates)
        list(GET _candidates -1 _wix_util_ext_path)
    endif()
endforeach()

if(NOT _wix_util_ext_path)
    message(WARNING
        "WixToolset.Util.wixext not found — desktop shortcut CA unavailable. "
        "Install it with:\n"
        "  wix extension add WixToolset.Util.wixext\n"
        "Then re-run cmake.")
    set(CPROJECT_WIX_AVAILABLE OFF)
endif()

endif()  # CPROJECT_WIX_AVAILABLE (extensions)

if(CPROJECT_WIX_AVAILABLE)

# ---- Inputs ----------------------------------------------------------------
set(CPROJECT_INSTALLER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/installer")
set(CPROJECT_INSTALLER_TEMPLATE_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/installer-staging/share/c-project-standard")

# Verify the .wxs sources exist before we wire up the targets. Failing here
# gives a clear "you are missing files" message instead of a WiX parser error
# 30 seconds later.
foreach(_src
    "${CPROJECT_INSTALLER_DIR}/Product.wxs"
    "${CPROJECT_INSTALLER_DIR}/Files.wxs"
    "${CPROJECT_INSTALLER_DIR}/Shortcuts.wxs"
    "${CPROJECT_INSTALLER_DIR}/License.rtf")
    if(NOT EXISTS "${_src}")
        message(FATAL_ERROR "Missing required WiX source: ${_src}")
    endif()
endforeach()

# ---- Output paths ----------------------------------------------------------
set(CPROJECT_MSI_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/msi")
file(MAKE_DIRECTORY "${CPROJECT_MSI_OUTPUT_DIR}")

set(CPROJECT_MSI_FILENAME
    "${PROJECT_NAME}-${PROJECT_VERSION}-win64.msi")
set(CPROJECT_MSI_PATH
    "${CPROJECT_MSI_OUTPUT_DIR}/${CPROJECT_MSI_FILENAME}")

set(CPROJECT_LIBRARY_FILE
    "${CMAKE_STATIC_LIBRARY_PREFIX}cproject_core${CMAKE_STATIC_LIBRARY_SUFFIX}")

# ---------------------------------------------------------------------------
# Target: install-staging
#
# Populates a clean staging directory under the build tree. We build the
# prerequisites (`cproject_core`, `cproject`) but not the `install` target
# itself - that target uses CMAKE_INSTALL_PREFIX (default: C:\Program Files
# on Windows) and would require elevation.
# ---------------------------------------------------------------------------
add_custom_target(install-staging
    COMMAND
        "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/installer-staging"
    COMMAND
        "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}"
            --target cproject_core cproject
    COMMAND
        "${CMAKE_COMMAND}" --install "${CMAKE_CURRENT_BINARY_DIR}"
            --prefix "${CMAKE_CURRENT_BINARY_DIR}/installer-staging"
        # Copy the PowerShell helper scripts into the staging bin directory.
        # These are used by the deferred custom actions in Shortcuts.wxs.
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different
                "${CPROJECT_INSTALLER_DIR}/create-shortcut.ps1"
                "${CMAKE_CURRENT_BINARY_DIR}/installer-staging/bin/create-shortcut.ps1"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different
                "${CPROJECT_INSTALLER_DIR}/remove-shortcut.ps1"
                "${CMAKE_CURRENT_BINARY_DIR}/installer-staging/bin/remove-shortcut.ps1"
        COMMENT "Staging install layout for MSI packaging"
    VERBATIM)

set(_template_out "${CMAKE_CURRENT_BINARY_DIR}/installer/generated/TemplateTree.wxs")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installer/generated")

# ---------------------------------------------------------------------------
# Target: msi
#
# Runs `wix build` to compile the four WiX sources into a single MSI.
#
# Preprocessor vars (-d X=value, accessed via $(var.CPROJECT.X) in the .wxs):
#   CPROJECT.Version           - PROJECT_VERSION
#   CPROJECT.SourceDir         - source root (used by WixUILicenseRtf)
#   CPROJECT.StagingDir        - root of install-staging
#   CPROJECT.StagingDirTemplate - install-staging/share/c-project-standard
#   CPROJECT.LibraryFileName   - staged static library filename
#
# The UI extension provides <ui:WixUI>; we must pass the dll path
# explicitly via -ext.
# ---------------------------------------------------------------------------
add_custom_target(msi
    COMMAND
        "${CMAKE_COMMAND}"
            "-DCPROJECT_STAGING_TEMPLATE_DIR=${CPROJECT_INSTALLER_TEMPLATE_DIR}"
            "-DCPROJECT_OUTPUT=${_template_out}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateWiXTemplateTree.cmake"
    COMMAND
        "${WIX_EXECUTABLE}" build
            "${CPROJECT_INSTALLER_DIR}/Product.wxs"
            "${CPROJECT_INSTALLER_DIR}/Files.wxs"
            "${CPROJECT_INSTALLER_DIR}/Shortcuts.wxs"
            "${_template_out}"
            -arch x64
            -bindpath "${CMAKE_CURRENT_BINARY_DIR}/installer-staging"
            -out "${CPROJECT_MSI_PATH}"
            "-d" "CPROJECT.Version=${PROJECT_VERSION}"
            "-d" "CPROJECT.SourceDir=${CMAKE_CURRENT_SOURCE_DIR}"
            "-d" "CPROJECT.StagingDir=${CMAKE_CURRENT_BINARY_DIR}/installer-staging"
            "-d" "CPROJECT.StagingDirTemplate=${CPROJECT_INSTALLER_TEMPLATE_DIR}"
            "-d" "CPROJECT.LibraryFileName=${CPROJECT_LIBRARY_FILE}"
            -pdbtype none
            -ext "${_wix_ui_ext_path}"
            -ext "${_wix_util_ext_path}"
    DEPENDS install-staging
        "${CPROJECT_INSTALLER_DIR}/Product.wxs"
        "${CPROJECT_INSTALLER_DIR}/Files.wxs"
        "${CPROJECT_INSTALLER_DIR}/Shortcuts.wxs"
        "${CPROJECT_INSTALLER_DIR}/License.rtf"
    COMMENT "Building ${CPROJECT_MSI_FILENAME}"
    VERBATIM)

# Expose the MSI path so consumers (CI, scripts) can find the artifact
# without scraping the build directory.
set(CPROJECT_MSI_PATH "${CPROJECT_MSI_PATH}" CACHE FILEPATH
    "Absolute path of the built MSI installer" FORCE)

message(STATUS "Windows MSI installer target 'msi' configured (WiX: ${WIX_EXECUTABLE})")
message(STATUS "  Output: ${CPROJECT_MSI_PATH}")
message(STATUS "  UI extension: ${_wix_ui_ext_path}")

endif()  # CPROJECT_WIX_AVAILABLE