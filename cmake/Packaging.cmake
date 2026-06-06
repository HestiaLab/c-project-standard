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
