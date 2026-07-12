function(cproject_add_test target)
    cmake_parse_arguments(CPROJECT_TEST
        ""
        ""
        "SOURCES;LIBS"
        ${ARGN}
    )

    if(NOT CPROJECT_TEST_SOURCES)
        message(FATAL_ERROR "cproject_add_test(${target}) requires SOURCES")
    endif()

    add_executable(${target}
        ${CPROJECT_TEST_SOURCES}
    )

    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
    )

    if(CPROJECT_TEST_LIBS)
        target_link_libraries(${target} PRIVATE ${CPROJECT_TEST_LIBS})
    endif()

    cproject_target_defaults(${target})

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    list(APPEND CPROJECT_TEST_TARGETS ${target})
    set(CPROJECT_TEST_TARGETS "${CPROJECT_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

set(CPROJECT_TEST_TARGETS "")

cproject_add_test(cproject_example_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_example.c
    LIBS
        cproject_core
)

# CPROJECT_WIZARD_TESTS_BEGIN
if(TARGET cproject)
    add_test(NAME cproject_help COMMAND $<TARGET_FILE:cproject> --help)
    set_tests_properties(cproject_help PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage:"
    )

    add_test(NAME cproject_version COMMAND $<TARGET_FILE:cproject> --version)
    set_tests_properties(cproject_version PROPERTIES
        PASS_REGULAR_EXPRESSION "[0-9]+\\.[0-9]+\\.[0-9]+"
    )

    add_test(NAME cproject_new_dry_run
        COMMAND $<TARGET_FILE:cproject>
            new SmokeProject
            --template-root ${CMAKE_CURRENT_SOURCE_DIR}
            --output ${CMAKE_CURRENT_BINARY_DIR}/cli-smoke
            --dry-run
            --yes
    )
    set_tests_properties(cproject_new_dry_run PROPERTIES
        PASS_REGULAR_EXPRESSION "dry-run"
    )
endif()
# CPROJECT_WIZARD_TESTS_END

if(WIN32)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME cproject_cpack_config
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check-cpack-config.py
            ${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake
    )
endif()

add_custom_target(cproject_tests
    DEPENDS
        ${CPROJECT_TEST_TARGETS}
)
