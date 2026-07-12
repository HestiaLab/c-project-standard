cmake_minimum_required(VERSION 3.21...3.31)

project(CProjectStandardPackageSmoke LANGUAGES C CXX)

find_package(CProjectStandard CONFIG REQUIRED)

function(copy_cproject_runtime target)
    if(NOT WIN32)
        return()
    endif()

    foreach(config_name IN ITEMS
            DEBUG
            RELEASE
            RELWITHDEBINFO
            MINSIZEREL
            NOCONFIG)
        get_target_property(runtime_location
            CProjectStandard::cproject_core
            "IMPORTED_LOCATION_${config_name}")
        if(runtime_location)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${runtime_location}"
                    "$<TARGET_FILE_DIR:${target}>"
                VERBATIM)
        endif()
    endforeach()
endfunction()

add_executable(package_smoke_c main.c)
target_link_libraries(package_smoke_c PRIVATE CProjectStandard::cproject_core)
copy_cproject_runtime(package_smoke_c)

add_executable(package_smoke_cxx cxx_header_smoke.cpp)
target_link_libraries(package_smoke_cxx PRIVATE CProjectStandard::cproject_core)
copy_cproject_runtime(package_smoke_cxx)

enable_testing()
add_test(NAME package_smoke_c COMMAND package_smoke_c)
add_test(NAME package_smoke_cxx COMMAND package_smoke_cxx)
