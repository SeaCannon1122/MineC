cmake_minimum_required(VERSION 3.15)

project(MineC)

add_subdirectory(shared/window)
add_subdirectory(shared/pixelchar)
add_subdirectory(shared/utility)
add_subdirectory(shared/networking)

add_subdirectory(testing)
add_subdirectory(client)