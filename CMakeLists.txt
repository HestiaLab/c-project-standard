cmake_minimum_required(VERSION 3.21...3.31)

project(CProjectStandard
    VERSION 0.2.0
    DESCRIPTION "A compact C11 project template and engineering standard"
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CPROJECT_IS_TOP_LEVEL_DEFAULT ON)
else()
    set(CPROJECT_IS_TOP_LEVEL_DEFAULT OFF)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(CPROJECT_ENABLE_ASAN "Enable AddressSanitizer where supported" OFF)
option(CPROJECT_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer where supported" OFF)
option(CPROJECT_ENABLE_COVERAGE "Enable coverage flags where supported" OFF)
option(CPROJECT_BUILD_EXAMPLE "Build the example executable" ${CPROJECT_IS_TOP_LEVEL_DEFAULT})
option(CPROJECT_BUILD_TESTING "Build CTest tests" ${CPROJECT_IS_TOP_LEVEL_DEFAULT})
option(CPROJECT_INSTALL "Generate install rules" ${CPROJECT_IS_TOP_LEVEL_DEFAULT})

set(CPROJECT_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cproject_version.h.in"
    "${CPROJECT_GENERATED_INCLUDE_DIR}/cproject/version.h"
    @ONLY
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerOptions.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sources.cmake")

add_library(cproject_core ${CPROJECT_CORE_SOURCES})
add_library(CProjectStandard::cproject_core ALIAS cproject_core)

target_compile_features(cproject_core PUBLIC c_std_11)

target_compile_definitions(cproject_core
    PRIVATE
        CPROJECT_BUILDING_LIBRARY
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(cproject_core PUBLIC CPROJECT_SHARED)
endif()
set_target_properties(cproject_core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(cproject_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CPROJECT_GENERATED_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)
cproject_target_defaults(cproject_core)

if(CPROJECT_BUILD_EXAMPLE)
    # Windows version-info resource
    if(WIN32)
        set(CPROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
        set(CPROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
        set(CPROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH})
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/res/cproject_example.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/generated/cproject.rc"
            @ONLY
        )
    endif()
    add_executable(cproject ${CPROJECT_EXAMPLE_SOURCES})
    if(WIN32)
        target_sources(cproject PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/generated/cproject.rc"
        )
    endif()
    target_link_libraries(cproject PRIVATE cproject_core)
    cproject_target_defaults(cproject)
    set_target_properties(cproject PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

if(CPROJECT_BUILD_TESTING)
    enable_testing()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Tests.cmake")
endif()

if(CPROJECT_INSTALL)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Windows uses a WiX-based MSI installer (see cmake/PackagingMSI.cmake).
        include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PackagingMSI.cmake")
    else()
        # Linux/macOS continue to use CPack (see cmake/Packaging.cmake).
        include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Packaging.cmake")
    endif()
endif()
