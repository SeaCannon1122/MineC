cmake_minimum_required(VERSION 3.15)
project(pixelchar)

add_library(pixelchar STATIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar_renderer.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pixelchar_font.c
)
target_include_directories(pixelchar 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Vulkan)
if (Vulkan_FOUND)
    
    add_library(pixelchar_backend_vulkan STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/vulkan/pixelchar_renderer_backend_vulkan.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/vulkan/pixelchar_font_backend_vulkan.c
    )

    target_include_directories(pixelchar_backend_vulkan 
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC  ${Vulkan_INCLUDE_DIRS}
    )
    
    target_compile_definitions(pixelchar_backend_vulkan 
        PUBLIC _PIXELCHAR_BACKEND_VULKAN=1
    )
    target_link_libraries(pixelchar_backend_vulkan PUBLIC ${Vulkan_LIBRARIES})

    target_link_libraries(pixelchar PUBLIC pixelchar_backend_vulkan)
endif()