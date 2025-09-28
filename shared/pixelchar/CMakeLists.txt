cmake_minimum_required(VERSION 3.15)
project(pixelchar)

add_library(pixelchar STATIC)

target_sources(pixelchar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar.c 
)
target_include_directories(pixelchar 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(pixelchar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/impl_vulkan.c
)
target_compile_definitions(pixelchar 
    PUBLIC PIXELCHAR_IMPLEMENTATION_VULKAN=1
 )

target_sources(pixelchar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/impl_opengl.c
)
target_compile_definitions(pixelchar 
    PUBLIC PIXELCHAR_IMPLEMENTATION_OPENGL=1
)