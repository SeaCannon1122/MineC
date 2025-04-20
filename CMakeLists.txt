cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(MineC)

add_subdirectory(shared/window)
add_subdirectory(shared/pixelchar)

add_subdirectory(testing)

add_subdirectory(client)