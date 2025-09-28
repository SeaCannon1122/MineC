cmake_minimum_required(VERSION 3.15)

project(Testing)

add_executable(direct3d12_testing "direct3d12_testing.c")
add_executable(general_testing general_testing.c)

target_link_libraries(direct3d12_testing PRIVATE cwindow d3d12 dxguid d3dcompiler dxgi utility)
target_link_libraries(general_testing PRIVATE utility)