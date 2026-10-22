# Template-only wizard library. This module is excluded from generated projects.
set(CPROJECT_WIZARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wizard/wizard_cli.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wizard/wizard_config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wizard/wizard_setup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wizard/wizard_util.c
)

add_library(cproject_wizard STATIC ${CPROJECT_WIZARD_SOURCES})
target_include_directories(cproject_wizard PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wizard
)
target_link_libraries(cproject_wizard PUBLIC cproject_core)
if(NOT WIN32)
    target_compile_definitions(cproject_wizard PRIVATE _POSIX_C_SOURCE=200809L)
endif()
cproject_target_defaults(cproject_wizard)
target_link_libraries(cproject PRIVATE cproject_wizard)
