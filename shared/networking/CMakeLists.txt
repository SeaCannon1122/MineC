cmake_minimum_required(VERSION 3.15)
project(networking)

add_library(networking STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/networking.c
)

target_include_directories(networking 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)