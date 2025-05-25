cmake_minimum_required(VERSION 3.15)
project(window)

add_library(window STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/window.c
)
target_include_directories(window 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    
    find_package(X11 REQUIRED)

    target_link_libraries(window 
        PUBLIC ${X11_LIBRARIES}
    )

endif()

target_compile_definitions(window
    PUBLIC _WINDOW_SUPPORT_VULKAN=1
)
target_include_directories(window 
    PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/Vulkan-Headers/include
)

target_compile_definitions(window
    PUBLIC _WINDOW_SUPPORT_OPENGL=1
)
target_include_directories(window 
    PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/OpenGL-Registry/api
)