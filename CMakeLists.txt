cmake_minimum_required(VERSION 3.20)

project(CProjectStandard
    VERSION 0.1.0
    DESCRIPTION "A compact C11 project template and engineering standard"
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(CPROJECT_ENABLE_ASAN "Enable AddressSanitizer where supported" OFF)
option(CPROJECT_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer where supported" OFF)
option(CPROJECT_ENABLE_COVERAGE "Enable coverage flags where supported" OFF)
option(CPROJECT_BUILD_EXAMPLE "Build the example executable" ON)

set(CPROJECT_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cproject_version.h.in"
    "${CPROJECT_GENERATED_INCLUDE_DIR}/cproject/version.h"
    @ONLY
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerOptions.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sources.cmake")

add_library(cproject_core STATIC ${CPROJECT_CORE_SOURCES})
target_include_directories(cproject_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CPROJECT_GENERATED_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)
cproject_target_defaults(cproject_core)

if(CPROJECT_BUILD_EXAMPLE)
    add_executable(cproject_example ${CPROJECT_EXAMPLE_SOURCES})
    target_link_libraries(cproject_example PRIVATE cproject_core)
    cproject_target_defaults(cproject_example)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Tests.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Packaging.cmake")
