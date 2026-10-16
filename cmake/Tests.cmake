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
# matching definition in src/. The Python interpreter is REQUIRED for testing:
# without it these gates would silently disappear from the CTest inventory and
# the suite would still report green with fewer tests.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    # Generic registration with absolute floors: valid for this template AND
    # for projects generated from it (whose scan surface is much smaller).
    add_test(NAME cproject_decl_def_check
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_decl_def.py
            ${CMAKE_CURRENT_SOURCE_DIR}
            --min-declarations 1
            --min-definitions 1)
    set_tests_properties(cproject_decl_def_check PROPERTIES LABELS "architecture")
else()
    message(FATAL_ERROR
        "Python3 interpreter not found, but CPROJECT_BUILD_TESTING is ON and "
        "the architecture gates require it. Install Python 3, or reconfigure "
        "with -DCPROJECT_BUILD_TESTING=OFF to build without the test suite.")
endif()

# Gate self-tests: assert the quality-gate scripts can actually fail. Pure
# stdlib unittest; requires the Python interpreter proven present above.
add_test(NAME cproject_gate_behaviors
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tests/test_gate_behaviors.py)
set_tests_properties(cproject_gate_behaviors PROPERTIES LABELS "architecture")

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

    # Version consistency: CMakeLists project VERSION, generated header,
    # CLI --version, and CHANGELOG newest section must all agree.
    add_test(NAME cproject_version_consistency
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_version_consistency.py
            ${CMAKE_CURRENT_SOURCE_DIR}
            --build-dir ${CMAKE_BINARY_DIR}
            --exe $<TARGET_FILE:cproject>
    )
    set_tests_properties(cproject_version_consistency PROPERTIES
        LABELS "architecture"
        DEPENDS cproject
    )

    # Template-only strict scan-surface floors (baseline x 0.8, measured
    # 2026-10-10). Registered inside the wizard markers on purpose: the wizard
    # comments this block out in generated projects, whose scan surface is
    # legitimately much smaller. Generated projects keep the generic
    # cproject_decl_def_check registration above with absolute floors.
    add_test(NAME cproject_decl_def_floor
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_decl_def.py
            ${CMAKE_CURRENT_SOURCE_DIR}
            --min-declarations 3
            --min-definitions 46
    )
    set_tests_properties(cproject_decl_def_floor PROPERTIES LABELS "architecture")

    # Documentation gate (template-maintenance): relative links/anchors
    # reachable (URL-decoded), code fences paired, and the README.md doc
    # index covers every docs/**/*.md file both ways. Registered inside the
    # markers because the wizard strips files the template index still
    # references; generated projects manage their own doc set.
    add_test(NAME cproject_docs_check
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_docs.py
            ${CMAKE_CURRENT_SOURCE_DIR}
            --mode all)
    set_tests_properties(cproject_docs_check PROPERTIES LABELS "documentation")

    # Backtick path reachability (git view) and CHANGELOG<->tag consistency.
    # Both require git evidence and fail with 无法取证 outside a git working
    # copy, so they live inside the wizard markers: generated projects are
    # not git repos at creation time.
    add_test(NAME cproject_doc_paths_check
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_doc_paths.py
            ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(cproject_doc_paths_check PROPERTIES LABELS "documentation")

    add_test(NAME cproject_changelog_tag_check
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_changelog_tag.py
            ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(cproject_changelog_tag_check PROPERTIES LABELS "documentation")
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
