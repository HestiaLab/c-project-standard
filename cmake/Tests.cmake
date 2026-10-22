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

# Generic version gate: CMake project VERSION, generated header, CHANGELOG,
# and the CLI when the project has one. Generated projects inherit this.
set(CPROJECT_VERSION_GATE_ARGS
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_version_consistency.py
    ${CMAKE_CURRENT_SOURCE_DIR}
    --build-dir ${CMAKE_BINARY_DIR}
)
if(TARGET cproject)
    list(APPEND CPROJECT_VERSION_GATE_ARGS --exe $<TARGET_FILE:cproject>)
endif()
add_test(NAME cproject_version_consistency
    COMMAND ${Python3_EXECUTABLE} ${CPROJECT_VERSION_GATE_ARGS}
)
set_tests_properties(cproject_version_consistency PROPERTIES LABELS "architecture")
if(TARGET cproject)
    set_tests_properties(cproject_version_consistency PROPERTIES DEPENDS cproject)
endif()

# Generic documentation gate: links, anchors, fences, and README index coverage.
# The generated README index and docs set are produced by the wizard together.
add_test(NAME cproject_docs_check
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_docs.py
        ${CMAKE_CURRENT_SOURCE_DIR}
        --mode all)
set_tests_properties(cproject_docs_check PROPERTIES LABELS "documentation")

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
        PASS_REGULAR_EXPRESSION "Dry run"
    )

    add_test(NAME cproject_cli_contract
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/wizard/test_cli_contract.py
            --exe $<TARGET_FILE:cproject>
            --template-root ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(cproject_cli_contract PROPERTIES
        LABELS "smoke"
        DEPENDS cproject
    )

    if(TARGET cproject_wizard)
        cproject_add_test(cproject_wizard_config_tests
            SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/wizard/test_wizard_config.c
            LIBS
                cproject_wizard
        )
    endif()

    # Template-only strict scan-surface floors (baseline x 0.8). Generated
    # projects have a legitimately smaller surface, so the absolute-floor
    # registration above is their guard.
    add_test(NAME cproject_decl_def_floor
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_decl_def.py
            ${CMAKE_CURRENT_SOURCE_DIR}
            --min-declarations 3
            --min-definitions 46
    )
    set_tests_properties(cproject_decl_def_floor PROPERTIES LABELS "architecture")

    # Backtick path reachability (git view) and CHANGELOG<->tag consistency.
    # Both require full git evidence and fail with 无法取证 outside a git
    # working copy, so they stay template-only.
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

# CPack config validation applies to generated projects with the installable
# feature and to the template's Linux/macOS packaging path. Windows template
# packaging uses WiX and does not produce CPackConfig.cmake.
# CPROJECT_FEATURE_INSTALLABLE_BEGIN
if(CPROJECT_INSTALL AND NOT (WIN32 AND CPROJECT_TEMPLATE))
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME cproject_cpack_config
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check-cpack-config.py
            ${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake
    )
endif()
# CPROJECT_FEATURE_INSTALLABLE_END

add_custom_target(cproject_tests
    DEPENDS
        ${CPROJECT_TEST_TARGETS}
)
