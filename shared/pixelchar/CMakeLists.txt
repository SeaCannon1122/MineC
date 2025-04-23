cmake_minimum_required(VERSION 3.15)
project(pixelchar)

add_library(pixelchar STATIC)

target_sources(pixelchar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar_renderer.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar_font.c
)
target_include_directories(pixelchar 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Vulkan)
if (Vulkan_FOUND)
    
    target_sources(pixelchar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/vulkan/pixelchar_renderer_backend_vulkan.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/vulkan/pixelchar_font_backend_vulkan.c
    )

    target_include_directories(pixelchar 
        PUBLIC  ${Vulkan_INCLUDE_DIRS}
    )
    
    target_compile_definitions(pixelchar 
        PUBLIC _PIXELCHAR_BACKEND_VULKAN=1
    )
    target_link_libraries(pixelchar PUBLIC ${Vulkan_LIBRARIES})

endif()

find_package(OpenGL)
if (OpenGL_FOUND)
    
    target_sources(pixelchar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/opengl/pixelchar_renderer_backend_opengl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/opengl/pixelchar_font_backend_opengl.c
        ${CMAKE_SOURCE_DIR}/dependencies/glad/src/glad.c
    )

    target_include_directories(pixelchar
        PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/glad/include
    )

    target_compile_definitions(pixelchar 
        PUBLIC _PIXELCHAR_BACKEND_OPENGL=1
    )

    target_link_libraries(pixelchar PUBLIC OpenGL::GL)
endif()

if(WIN32)

    target_sources(pixelchar PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/directx/pixelchar_renderer_backend_directx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/directx/pixelchar_font_backend_directx.cpp
    )


    target_compile_definitions(pixelchar 
        PUBLIC _PIXELCHAR_BACKEND_DIRECTX=1
    )

    target_link_libraries(pixelchar PUBLIC d3d12 dxguid d3dcompiler)

endif()