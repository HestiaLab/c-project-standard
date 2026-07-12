include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---------------------------------------------------------------------------
# Install destinations
# ---------------------------------------------------------------------------
set(CPROJECT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/CProjectStandard")
set(CPROJECT_INSTALL_DATADIR "${CMAKE_INSTALL_DATADIR}/c-project-standard")

# ---------------------------------------------------------------------------
# Library + headers + CMake config
# ---------------------------------------------------------------------------
install(TARGETS cproject_core
    EXPORT CProjectStandardTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CPROJECT_GENERATED_INCLUDE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CProjectStandardConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CProjectStandardConfig.cmake"
    INSTALL_DESTINATION "${CPROJECT_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CProjectStandardConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(EXPORT CProjectStandardTargets
    NAMESPACE CProjectStandard::
    DESTINATION ${CPROJECT_INSTALL_CMAKEDIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CProjectStandardConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CProjectStandardConfigVersion.cmake"
    DESTINATION ${CPROJECT_INSTALL_CMAKEDIR}
)

# ---------------------------------------------------------------------------
# Top-level documentation
# ---------------------------------------------------------------------------
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# ---------------------------------------------------------------------------
# Interactive wizard executable + launcher
# ---------------------------------------------------------------------------
if(CPROJECT_BUILD_EXAMPLE AND TARGET cproject)
    install(TARGETS cproject
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/run-wizard.bat"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# ---------------------------------------------------------------------------
# Template source files (needed by the interactive setup wizard)
# ---------------------------------------------------------------------------
# Install the entire source tree excluding build artifacts and VCS directories.
# Without FILES_MATCHING, PATTERN matches directory names (excluding the whole
# subtree) and file patterns (excluding individual files).
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CPROJECT_INSTALL_DATADIR}
    PATTERN "build" EXCLUDE
    PATTERN ".git" EXCLUDE
    PATTERN ".vs" EXCLUDE
    PATTERN ".cache" EXCLUDE
    PATTERN "install" EXCLUDE
    PATTERN "install-*" EXCLUDE
    PATTERN "dist" EXCLUDE
    PATTERN "out" EXCLUDE
    PATTERN "_CPack_Packages" EXCLUDE
    PATTERN "Testing" EXCLUDE
    PATTERN "*.exe" EXCLUDE
    PATTERN "*.o" EXCLUDE
    PATTERN "*.obj" EXCLUDE
    PATTERN "*.a" EXCLUDE
    PATTERN "*.lib" EXCLUDE
    PATTERN "*.so" EXCLUDE
    PATTERN "*.dylib" EXCLUDE
    PATTERN "*.dll" EXCLUDE
    PATTERN ".ninja_*" EXCLUDE
    PATTERN "*.ninja" EXCLUDE
    PATTERN "compile_commands.json" EXCLUDE
    PATTERN "CTestTestfile.cmake" EXCLUDE
    PATTERN "cmake_install.cmake" EXCLUDE
    PATTERN "CMakeCache.txt" EXCLUDE
    PATTERN "InstallScripts.json" EXCLUDE
    PATTERN "install_manifest.txt" EXCLUDE
    PATTERN "cmake.check_cache" EXCLUDE
    PATTERN "TargetDirectories.txt" EXCLUDE
    PATTERN "*.idx" EXCLUDE
    REGEX "\\\\.swp$" EXCLUDE
    REGEX "\\\\.swo$" EXCLUDE
    REGEX "\\\\.user$" EXCLUDE
)

# ---------------------------------------------------------------------------
# CPack — platform-specific binary + source packages
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CProjectStandard")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "CProjectStandard contributors")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
endif()
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/cproject.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/res/cproject.ico")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
    "CreateShortCut \"$DESKTOP\\CProjectStandard.lnk\" \"$INSTDIR\\bin\\run-wizard.bat\"")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
    "Delete \"$DESKTOP\\CProjectStandard.lnk\"")

# --- cross-platform default generators ---
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_HELP_LINK "https://github.com/wubing7755/c-project-standard")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/wubing7755/c-project-standard")
    set(CPACK_NSIS_CONTACT "CProjectStandard contributors")
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_MENU_LINKS
        "bin/run-wizard.bat" "Project Setup Wizard"
        "share/c-project-standard" "Template Files"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
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
