# 测试注册辅助函数。
#
# 每个测试源文件生成一个独立可执行文件并注册到 CTest。
# 测试断言使用 Unity（vendored 到 tests/vendor/unity/，MIT）。

# Unity vendored 源码（编译一次，供所有测试 target 复用）。
set(CPROJECT_UNITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/vendor/unity/unity.c
)

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
        ${CPROJECT_UNITY_SOURCES}
    )

    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/vendor/unity
    )

    if(CPROJECT_TEST_LIBS)
        target_link_libraries(${target} PRIVATE ${CPROJECT_TEST_LIBS})
    endif()

    cproject_target_defaults(${target})

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    set_tests_properties(${target} PROPERTIES LABELS "unit")
    list(APPEND CPROJECT_TEST_TARGETS ${target})
    set(CPROJECT_TEST_TARGETS "${CPROJECT_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

set(CPROJECT_TEST_TARGETS "")

# Architecture gate: every public function declaration in include/ must have a
# matching definition in src/. Optional: skipped when a Python interpreter is
# not available on the host.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME cproject_decl_def_check
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_decl_def.py
            ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(cproject_decl_def_check PROPERTIES LABELS "architecture")
endif()

cproject_add_test(cproject_example_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_example.c
    LIBS
        cproject_core
)

cproject_add_test(cproject_scalar_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_scalar.c
)

# CPROJECT_WIZARD_TESTS_BEGIN
if(TARGET cproject)
    add_test(NAME cproject_help COMMAND $<TARGET_FILE:cproject> --help)
    set_tests_properties(cproject_help PROPERTIES
        LABELS "smoke"
        PASS_REGULAR_EXPRESSION "Usage:"
    )

    add_test(NAME cproject_version COMMAND $<TARGET_FILE:cproject> --version)
    set_tests_properties(cproject_version PROPERTIES
        LABELS "smoke"
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
        LABELS "smoke"
        PASS_REGULAR_EXPRESSION "dry-run"
    )
endif()
# CPROJECT_WIZARD_TESTS_END

# CPack config validation is only meaningful on non-Windows platforms where
# we still use CPack for packaging (DEB/RPM/TGZ on Linux, productbuild/TGZ
# on macOS). On Windows the packaging is done via WiX 6 MSI.
if(NOT WIN32)
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
