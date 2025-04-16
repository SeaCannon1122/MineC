cmake_minimum_required(VERSION 3.15)

project(MineC_Client)

add_subdirectory(../shared/pixelchar)
include_directories(../shared/pixelchar/include)

set(SOURCE_FILES

    src/main.c
    src/minec_client.c
    src/application/application.c
    src/application/application_window.c
    src/resources/resources.c
    src/settings/settings.c
    src/simulator/simulator.c
    src/networker/networker.c
    src/renderer/renderer.c
    src/renderer/backend/renderer_backend.c
    src/gui/gui.c
    src/gui/gui_menus.c

    ../shared/general/src/key_value.c
    ../shared/general/src/resource_manager.c
    ../shared/general/src/string_allocator.c
    ../shared/general/src/platformlib/platform/platform.c
    ../shared/general/src/platformlib/networking/networking.c
)

set(RENDERER_BACKEND_VULKAN_SOURCE_FILES
    src/renderer/backend/vulkan/vulkan_backend.c
)

find_package(Vulkan REQUIRED)

include_directories(
    src
    ../dependencies/stb_image/include
    ${Vulkan_INCLUDE_DIRS}
)

link_libraries(${Vulkan_LIBRARIES})

add_compile_definitions(CLIENT_GRAPHICS_API_VULKAN=1)
add_compile_definitions(CLIENT_GRAPHICS_API_OPENGL=1)
add_compile_definitions(CLIENT_GRAPHICS_API_DIRECT=1)

add_executable(minec_client ${SOURCE_FILES})
add_library(minec_client_renderer_backend_vulkan ${RENDERER_BACKEND_VULKAN_SOURCE_FILES})

target_link_libraries(minec_client PRIVATE blocks_client_vulkan_backend pixelchar)

if(WIN32)

    set_target_properties(minec_client PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

elseif(UNIX)

    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    target_link_libraries(minec_client PUBLIC X11 pthread m)
   
endif()

add_custom_command(TARGET minec_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/runtime_files"
    "${CMAKE_BINARY_DIR}/runtime_files"
)