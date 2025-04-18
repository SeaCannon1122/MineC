cmake_minimum_required(VERSION 3.15)

project(Testing)

find_package(Vulkan REQUIRED)

set(TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.c
    ${CMAKE_SOURCE_DIR}/shared/general/src/hashmap.c

    ${CMAKE_SOURCE_DIR}/shared/general/src/string_allocator.c
)

set(TEST_2_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test2/main.c

    ${CMAKE_SOURCE_DIR}/client/src/application_window/application_window.c

    ${CMAKE_SOURCE_DIR}/shared/general/src/platformlib/platform/platform.c
)

add_executable(minec_test ${TEST_SOURCE_FILES})
add_executable(minec_test2 ${TEST_2_SOURCE_FILES})

target_include_directories(minec_test2
    PRIVATE
        ${CMAKE_SOURCE_DIR}/shared/general/src
        ${CMAKE_SOURCE_DIR}/shared/pixelchar/include
        ${CMAKE_SOURCE_DIR}/client/src
)

target_include_directories(minec_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/shared/general/src
        ${CMAKE_SOURCE_DIR}/shared/pixelchar/include
        ${CMAKE_SOURCE_DIR}/client/src
)

target_link_libraries(minec_test2 PRIVATE pixelchar)