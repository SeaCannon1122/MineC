cmake_minimum_required(VERSION 3.10)
project(stb_image)

add_library(stb_image STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image.c)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UNIX)
    target_link_libraries(stb_image PRIVATE m)
endif()