cmake_minimum_required(VERSION 3.15)

project(Testing)

find_package(Vulkan REQUIRED)

set(TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.c
    ${CMAKE_SOURCE_DIR}/dependencies/glad/src/glad.c
)

set(TEST_2_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test2/main.c
)

add_executable(minec_test ${TEST_SOURCE_FILES})
add_executable(minec_test2 ${TEST_2_SOURCE_FILES})
add_executable(minec_test3  ${CMAKE_CURRENT_SOURCE_DIR}/test3/main.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    
    find_package(X11 REQUIRED)

    target_link_libraries(minec_test2 
        PUBLIC ${X11_LIBRARIES}
    )

endif()

target_link_libraries(minec_test2 PRIVATE pixelchar window)
target_link_libraries(minec_test3 PRIVATE window d3d12 dxguid d3dcompiler dxgi)
target_link_libraries(minec_test PRIVATE pixelchar window)

add_custom_target(test_copy_runtime_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/client/runtime_files"
    "${CMAKE_CURRENT_BINARY_DIR}/runtime_files"
    COMMENT "Copying runtime files..."
)

add_dependencies(minec_test2 test_copy_runtime_files)
add_dependencies(minec_test test_copy_runtime_files)