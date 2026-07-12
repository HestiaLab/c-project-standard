# Install rules shared across all platforms.
#
# NOTE: On Windows this module is NOT included (see CMakeLists.txt).
# Windows packaging uses cmake/PackagingMSI.cmake with WiX instead.
# Include this file only on Linux and macOS.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallRules.cmake")

# ---------------------------------------------------------------------------
# CPack — binary + source packages (Linux / macOS only)
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CProjectStandard")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "CProjectStandard contributors")

# --- platform-specific generators ---
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CPACK_GENERATOR "productbuild;TGZ")
    set(CPACK_PRODUCTBUILD_IDENTIFIER "com.cprojectstandard.template")
else()
    # Linux — prefer DEB+RPM when build tools are available; always include TGZ
    find_program(CPACK_DEB_EXECUTABLE dpkg)
    find_program(CPACK_RPM_EXECUTABLE rpmbuild)
    set(CPACK_GENERATOR "TGZ")
    if(CPACK_DEB_EXECUTABLE)
        list(APPEND CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "CProjectStandard contributors")
        set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
        set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    endif()
    if(CPACK_RPM_EXECUTABLE)
        list(APPEND CPACK_GENERATOR "RPM")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
    endif()
endif()

# --- source package ---
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "/_CPack_Packages/"
    "/\\\\.git/"
    "/\\\\.vs/"
    "/\\\\.cache/"
    "/install/"
    "/install-"
    "/dist/"
    "/out/"
    "\\\\.swp$"
    "\\\\.swo$"
    "\\\\.user$"
    "\\\\.idx$"
)

include(CPack)