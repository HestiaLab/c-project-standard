cmake_minimum_required(VERSION 3.21)

project(CProjectStandardSubprojectSmoke LANGUAGES C)

if(NOT DEFINED CPROJECT_SOURCE_DIR)
    message(FATAL_ERROR "CPROJECT_SOURCE_DIR must point at the template source tree")
endif()

add_subdirectory("${CPROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/cproject-standard")

if(NOT TARGET CProjectStandard::cproject_core)
    message(FATAL_ERROR "Expected CProjectStandard::cproject_core alias target")
endif()

foreach(option_name IN ITEMS
        CPROJECT_BUILD_EXAMPLE
        CPROJECT_BUILD_TESTING
        CPROJECT_INSTALL)
    if(${option_name})
        message(FATAL_ERROR "${option_name} should default to OFF for subproject builds")
    endif()
endforeach()

foreach(target_name IN ITEMS
        cproject_example
        cproject_example_tests
        cproject_tests)
    if(TARGET ${target_name})
        message(FATAL_ERROR "${target_name} should not be created by default for subproject builds")
    endif()
endforeach()

add_executable(subproject_smoke main.c)
target_link_libraries(subproject_smoke PRIVATE CProjectStandard::cproject_core)

enable_testing()
add_test(NAME subproject_smoke COMMAND subproject_smoke)
