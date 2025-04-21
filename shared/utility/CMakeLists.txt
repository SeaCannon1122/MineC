cmake_minimum_required(VERSION 3.15)
project(utility)

add_library(utility STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hashmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string_allocator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
)

target_include_directories(utility 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)