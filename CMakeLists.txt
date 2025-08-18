cmake_minimum_required(VERSION 3.15)

project(MineC)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_C_STANDARD 11)
else()
    set(CMAKE_C_STANDARD 99)
endif()

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(dependencies/graphics_headers)

add_subdirectory(dependencies/stb_image)

add_subdirectory(shared/window)
add_subdirectory(shared/pixelchar)
add_subdirectory(shared/utility)
add_subdirectory(shared/networking)

add_subdirectory(testing)
add_subdirectory(client)