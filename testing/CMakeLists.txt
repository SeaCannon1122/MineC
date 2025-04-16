cmake_minimum_required(VERSION 3.15)

add_subdirectory(../shared/pixelchar ${CMAKE_BINARY_DIR}/shared/pixelchar)
include_directories(../shared/pixelchar/include)

project(Testing)

find_package(Vulkan REQUIRED)

include_directories(
    ../shared/general/src
    ../client/src
)

set(TEST_SOURCE_FILES
    test/main.c

    ../shared/general/src/string_allocator.c
)

set(TEST_2_SOURCE_FILES
    test2/main.c

    ../client/src/application/application_window.c

    ../shared/general/src/platformlib/platform/platform.c
)

add_executable(minec_test ${TEST_SOURCE_FILES})
add_executable(minec_test2 ${TEST_2_SOURCE_FILES})

target_link_libraries(minec_test2 PRIVATE pixelchar)