cmake_minimum_required(VERSION 3.15)
project(blocks2)

set(CLIENT_SOURCE_FILES

    src/client/main.c
    src/client/game_client.c
    src/client/application/application.c
    src/client/resources/resources.c
    src/client/settings/settings.c
    src/client/simulator/simulator.c
    src/client/networker/networker.c
    src/client/renderer/renderer.c
    src/client/renderer/backend/renderer_backend.c
    src/client/gui/gui.c
    src/client/gui/gui_menus.c

    src/general/pixelchar/pixelchar.c
    src/general/key_value.c
    src/general/resource_manager.c
    src/general/string_allocator.c
    src/general/platformlib/platform/platform.c
    src/general/platformlib/networking/networking.c

)

set(SERVER_SOURCE_FILES
    src/server/main.c
    src/server/game_server.c
    src/server/resources/resources.c
    src/server/networker/networker.c
    src/server/settings/settings.c

    src/general/key_value.c
    src/general/resource_manager.c
    src/general/platformlib/platform/platform.c
    src/general/platformlib/networking/networking.c
)

set(TEST_SOURCE_FILES
    src/test/main.c

    src/general/string_allocator.c
)

set(TEST_2_SOURCE_FILES
    src/test2/main.c

    src/general/platformlib/platform/platform.c
    src/general/platformlib/networking/networking.c
)

find_package(Vulkan REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/dependencies/stb_image/include
    ${Vulkan_INCLUDE_DIRS}
)

link_libraries(${Vulkan_LIBRARIES})

add_executable(blocks_client ${CLIENT_SOURCE_FILES})
add_executable(blocks_server ${SERVER_SOURCE_FILES})
add_executable(blocks_test ${TEST_SOURCE_FILES})
add_executable(blocks_test2 ${TEST_2_SOURCE_FILES})

add_compile_definitions(CLIENT_GRAPHICS_API_VULKAN=1)

if(WIN32)

    set_target_properties(blocks_client PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

elseif(UNIX)

    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    target_link_libraries(blocks_client X11 pthread m)
    target_link_libraries(blocks_test2 X11 pthread m)
    
endif()


#copying resource files
add_custom_command(TARGET blocks_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "${CMAKE_BINARY_DIR}/resources"
)

add_custom_command(TARGET blocks_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "${CMAKE_BINARY_DIR}/resources"
)
