cmake_minimum_required(VERSION 3.15)

project(Testing)

add_executable(pixelchar_test_opengl pixelchar_test_opengl.c)
add_executable(pixelchar_test_vulkan pixelchar_test_vulkan.c)
add_executable(direct3d12_testing direct3d12_testing.cpp)
add_executable(old_to_new_pixelfont old_to_new_pixelfont.c)
add_executable(general_testing general_testing.c)

target_link_libraries(pixelchar_test_vulkan PRIVATE pixelchar window utility)
target_link_libraries(direct3d12_testing PRIVATE window d3d12 dxguid d3dcompiler dxgi utility)
target_link_libraries(pixelchar_test_opengl PRIVATE pixelchar window utility)
target_link_libraries(general_testing PRIVATE utility)

add_custom_target(test_copy_runtime_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/client/runtime_files"
    "${CMAKE_CURRENT_BINARY_DIR}/runtime_files"
    COMMENT "Copying runtime files..."
)

add_dependencies(pixelchar_test_opengl test_copy_runtime_files)
add_dependencies(pixelchar_test_vulkan test_copy_runtime_files)