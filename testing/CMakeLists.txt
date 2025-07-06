cmake_minimum_required(VERSION 3.15)

project(Testing)

add_executable(pixelchar_test_opengl pixelchar_test_opengl.c)
add_executable(pixelchar_test_vulkan pixelchar_test_vulkan.c)
add_executable(direct3d12_testing direct3d12_testing.cpp)
add_executable(old_to_new_pixelfont old_to_new_pixelfont.c)
add_executable(general_testing general_testing.c)
add_executable(cerialize_test cerialize_test.c ../client/.cerialized_src/resources.c)
target_include_directories(cerialize_test PRIVATE ../client/.cerialized_src/)

find_package(Vulkan)

target_link_libraries(pixelchar_test_vulkan PRIVATE pixelchar window utility ${Vulkan_LIBRARIES})
target_link_libraries(direct3d12_testing PRIVATE window d3d12 dxguid d3dcompiler dxgi utility)
target_link_libraries(pixelchar_test_opengl PRIVATE pixelchar window utility)
target_link_libraries(general_testing PRIVATE utility)
target_link_libraries(cerialize_test PRIVATE utility)