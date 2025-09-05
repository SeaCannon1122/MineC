cmake_minimum_required(VERSION 3.15)

project(MineC)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(dependencies/graphics_headers)

add_subdirectory(dependencies/stb_image)

add_subdirectory(shared/cwindow)
add_subdirectory(shared/pixelchar)
add_subdirectory(shared/utility)
add_subdirectory(shared/networking)

add_subdirectory(testing)
add_subdirectory(client)