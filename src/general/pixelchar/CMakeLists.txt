cmake_minimum_required(VERSION 3.15)
project(pixelchar)

include_directories(src)

add_library(pixelchar STATIC "src/pixelchar.c" "src/pixelchar_renderer.c" "src/pixelfont.c")

find_package(Vulkan)
if (Vulkan_FOUND)
    
    add_library(pixelchar_backend_vulkan STATIC src/backend/vulkan/vulkan_backend.c)
    target_compile_definitions(pixelchar_backend_vulkan PUBLIC _PIXELCHAR_BACKEND_VULKAN=1)
    target_compile_definitions(pixelchar PUBLIC _PIXELCHAR_BACKEND_VULKAN=1)
    target_include_directories(pixelchar_backend_vulkan PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(pixelchar_backend_vulkan PRIVATE ${Vulkan_LIBRARIES})

    target_link_libraries(pixelchar PRIVATE pixelchar_backend_vulkan)
endif()