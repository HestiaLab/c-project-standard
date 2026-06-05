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

cproject_add_test(cproject_example_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_example.c
    LIBS
        cproject_core
)

add_custom_target(cproject_tests
    DEPENDS
        ${CPROJECT_TEST_TARGETS}
)
