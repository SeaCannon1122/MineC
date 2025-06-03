cmake_minimum_required(VERSION 3.10)
project(window)

add_library(window STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/window.c)
target_include_directories(window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (WIN32)
    set(WINDOW_GRAPHICS_VULKAN_CMAKE ON)
    set(WINDOW_GRAPHICS_OPENGL_CMAKE ON)

    target_compile_definitions(window PUBLIC WINDOW_IMPLEMENTATION_WINDOWS)

elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)

    set(WINDOW_GRAPHICS_VULKAN_CMAKE ON)
    set(WINDOW_GRAPHICS_OPENGL_CMAKE ON)

    target_compile_definitions(window PUBLIC WINDOW_IMPLEMENTATION_X11)

    target_link_libraries(window PUBLIC ${X11_LIBRARIES})

elseif(UNIX AND APPLE)

    target_compile_definitions(window PUBLIC WINDOW_IMPLEMENTATION_MACOS)

endif()

if (WINDOW_GRAPHICS_VULKAN_CMAKE)
    target_compile_definitions(window PUBLIC WINDOW_GRAPHICS_VULKAN=1 )
    target_include_directories(window PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/Vulkan-Headers/include )
endif()

if (WINDOW_GRAPHICS_OPENGL_CMAKE)
    target_compile_definitions(window PUBLIC WINDOW_GRAPHICS_OPENGL=1 )
    target_include_directories(window PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/OpenGL-Registry/api )
endif()