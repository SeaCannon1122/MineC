cmake_minimum_required(VERSION 3.15)
project(window)

add_library(window STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/window.c
)
target_include_directories(window 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/window
)

find_package(Vulkan)
if (Vulkan_FOUND)

    target_compile_definitions(window
        PUBLIC _WINDOW_SUPPORT_VULKAN=1
    )

    target_include_directories(window 
        PUBLIC  ${Vulkan_INCLUDE_DIRS}
    )

    target_link_libraries(window 
        PUBLIC ${Vulkan_LIBRARIES}
    )

endif()

find_package(OpenGL)
if (OpenGL_FOUND)

    target_compile_definitions(window
        PUBLIC _WINDOW_SUPPORT_OPENGL=1
    )

    target_link_libraries(window 
        PUBLIC OpenGL::GL
    )

endif()