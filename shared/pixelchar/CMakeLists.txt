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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/backend_vulkan.c
)
target_compile_definitions(pixelchar 
    PUBLIC _PIXELCHAR_BACKEND_VULKAN=1
)

target_sources(pixelchar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/backend_opengl.c
)
target_compile_definitions(pixelchar 
    PUBLIC _PIXELCHAR_BACKEND_OPENGL=1
)



if(WIN32)

    target_sources(pixelchar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/backend_direct3d12.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/backend_direct3d11.cpp
    )

    target_compile_definitions(pixelchar 
        PUBLIC 
            _PIXELCHAR_BACKEND_DIRECT3D11=1
            _PIXELCHAR_BACKEND_DIRECT3D12=1
    )

endif()