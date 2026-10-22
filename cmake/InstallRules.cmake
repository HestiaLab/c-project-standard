# ---------------------------------------------------------------------------
# Cross-platform install rules.
#
# Included by:
#   * cmake/Packaging.cmake    (Linux/macOS and generated projects)
#   * cmake/PackagingMSI.cmake (Windows template MSI)
#
# The library/package layout is common to the template and generated projects.
# Wizard binaries and the template payload are template-only.
# ---------------------------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CPROJECT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/CProjectStandard")

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

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
# CPROJECT_TEMPLATE_ONLY_BEGIN
if(NOT CPROJECT_TEMPLATE)
# CPROJECT_TEMPLATE_ONLY_END
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )
# CPROJECT_TEMPLATE_ONLY_BEGIN
endif()
# CPROJECT_TEMPLATE_ONLY_END

if(TARGET cproject)
    install(TARGETS cproject
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# CPROJECT_TEMPLATE_ONLY_BEGIN
if(CPROJECT_TEMPLATE)
    if(TARGET cproject)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/run-wizard.bat"
            DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    # The installed template must contain the payload the wizard copies, but
    # never template-only sources. template-exclude.txt is the single manifest
    # shared with the wizard's copy_tree implementation.
    set(CPROJECT_INSTALL_DATADIR "${CMAKE_INSTALL_DATADIR}/c-project-standard")
    set(_template_exclude_args "")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/template-exclude.txt")
        file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/template-exclude.txt" _template_excludes)
        foreach(_entry IN LISTS _template_excludes)
            string(STRIP "${_entry}" _entry)
            if(_entry STREQUAL "" OR _entry MATCHES "^#")
                continue()
            endif()
            string(REGEX REPLACE "/+$" "" _entry "${_entry}")
            if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_entry}")
                # install(DIRECTORY ... PATTERN) matches directory *names*, not
                # slash-qualified directory paths.
                get_filename_component(_entry_name "${_entry}" NAME)
                list(APPEND _template_exclude_args PATTERN "${_entry_name}" EXCLUDE)
            else()
                list(APPEND _template_exclude_args PATTERN "${_entry}" EXCLUDE)
            endif()
        endforeach()
    endif()

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${CPROJECT_INSTALL_DATADIR}
        ${_template_exclude_args}
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
        PATTERN "__pycache__" EXCLUDE
        PATTERN "*.pyc" EXCLUDE
        PATTERN "*-cppcheck-build-dir" EXCLUDE
        PATTERN ".static-analysis-files.txt" EXCLUDE
        PATTERN "README.md" EXCLUDE
        PATTERN "CMakeUserPresets.json" EXCLUDE
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
    # The template's root README is maintainer-facing; generated projects get
    # a fresh project README. The ADR index is part of the shared docs.
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/adr/README.md"
        DESTINATION "${CPROJECT_INSTALL_DATADIR}/docs/adr"
    )
endif()
# CPROJECT_TEMPLATE_ONLY_END
