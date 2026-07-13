# ---------------------------------------------------------------------------
# Cross-platform install rules.
#
# Included by:
#   * cmake/Packaging.cmake  (Linux/macOS — followed by include(CPack))
#   * cmake/PackagingMSI.cmake (Windows — followed by WiX build targets)
#
# Defines the on-disk layout for `cmake --install` and for the MSI staging
# directory. Keeping these rules separate from CPack/NSIS configuration lets
# the Windows build use WiX without inheriting CPack variables, and vice
# versa.
# ---------------------------------------------------------------------------

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
    PATTERN ".hermes" EXCLUDE
    PATTERN "installer" EXCLUDE
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
