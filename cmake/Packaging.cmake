# Install rules shared across platforms.
#
# On the template's Windows build, cmake/PackagingMSI.cmake is used instead.
# Generated projects use this CPack path on every platform: the template MSI
# and its installer sources are deliberately template-only.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallRules.cmake")

string(TOLOWER "${PROJECT_NAME}" CPROJECT_PROJECT_ID)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "${CPROJECT_PROJECT_AUTHOR}")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # CPack's productbuild generator can emit an empty distribution on current
    # CMake versions. The release workflow wraps the staged install into a
    # pkgbuild/productbuild package instead; local cpack produces the TGZ.
    set(CPACK_GENERATOR "TGZ")
else()
    find_program(CPACK_DEB_EXECUTABLE dpkg)
    find_program(CPACK_RPM_EXECUTABLE rpmbuild)
    set(CPACK_GENERATOR "TGZ")
    if(CPACK_DEB_EXECUTABLE)
        list(APPEND CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPROJECT_PROJECT_AUTHOR}")
        set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
        set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    endif()
    if(CPACK_RPM_EXECUTABLE)
        list(APPEND CPACK_GENERATOR "RPM")
        set(CPACK_RPM_PACKAGE_LICENSE "${CPROJECT_PROJECT_LICENSE}")
        set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
    endif()
endif()

set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "/_CPack_Packages/"
    "/\\\\.git/"
    "/\\\\.vs/"
    "/\\\\.cache/"
    "/\\\\.hermes/"
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
