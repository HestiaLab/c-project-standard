cmake_minimum_required(VERSION 3.20)

project(CProjectStandardPackageSmoke LANGUAGES C CXX)

find_package(CProjectStandard CONFIG REQUIRED)

add_executable(package_smoke_c main.c)
target_link_libraries(package_smoke_c PRIVATE CProjectStandard::cproject_core)

add_executable(package_smoke_cxx cxx_header_smoke.cpp)
target_link_libraries(package_smoke_cxx PRIVATE CProjectStandard::cproject_core)

enable_testing()
add_test(NAME package_smoke_c COMMAND package_smoke_c)
add_test(NAME package_smoke_cxx COMMAND package_smoke_cxx)
