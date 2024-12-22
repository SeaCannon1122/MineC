cmake_minimum_required(VERSION 3.15)
project(blocks2)

set(CLIENT_SOURCE_FILES

    src/client/main.c
    src/client/game_client.c
    src/client/application/application.c
    src/client/resources/resources.c
    src/client/resources/settings.c
    src/client/simulator/simulator.c
    src/client/simulator/networker.c
    src/client/renderer/renderer.c
    src/client/renderer/backend/renderer_backend.c

    src/general/resource_manager.c
    src/general/platformlib/platform/platform.c
    src/general/platformlib/networking.c
    src/general/rendering/rendering_memory_manager.c
    src/general/rendering/vulkan_helpers.c
    src/general/rendering/rendering_window.c
    src/general/rendering/gui/pixel_char.c
    src/general/key_value.c

)

set(SERVER_SOURCE_FILES
    src/server/main.c
    src/server/game_server_body/game_server.c
    src/server/game_server_body/game_simulator.c

    src/general/platformlib/platform/platform.c
    src/general/platformlib/networking.c
    src/general/key_value.c
)

set(TEST_SOURCE_FILES
    src/test/main.c
    src/general/resource_manager.c
    src/general/rendering/vulkan_helpers.c
    src/general/rendering/rendering_window.c
    src/general/rendering/rendering_memory_manager.c
    src/general/rendering/gui/pixel_char.c

    src/general/platformlib/platform/platform.c
    src/general/key_value.c

)

set(TEST_2_SOURCE_FILES
    src/test2/main.c
    src/test2/testing2.c
    src/general/rendering/vulkan_helpers.c
    src/general/rendering/rendering_window.c
    src/general/rendering/rendering_memory_manager.c
    src/general/rendering/gui/pixel_char.c

    src/general/platformlib/platform/platform.c

)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
link_libraries(${Vulkan_LIBRARIES})

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/stb_image/include)

add_executable(blocks_client ${CLIENT_SOURCE_FILES})
add_executable(blocks_server ${SERVER_SOURCE_FILES})
add_executable(blocks_test ${TEST_SOURCE_FILES})
add_executable(blocks_test2 ${TEST_2_SOURCE_FILES})

add_compile_definitions(CLIENT_GRAPHICS_API_VULKAN=1)

if(WIN32)

    set_target_properties(blocks_client PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    set_target_properties(blocks_test PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    set_target_properties(blocks_test2 PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    target_link_libraries(blocks_client)
    target_link_libraries(blocks_test)
    target_link_libraries(blocks_test2)

elseif(UNIX)
    find_package(X11 REQUIRED)
    
    include_directories(${X11_INCLUDE_DIR})

    target_link_libraries(blocks_client X11 pthread m)
    target_link_libraries(blocks_test X11 pthread m)
    target_link_libraries(blocks_test2 X11 pthread m)
    target_link_libraries(blocks_server X11 pthread m)
endif()

set(RESOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/resources")
set(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

add_custom_command(TARGET blocks_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIRECTORY}"
    "${OUTPUT_DIRECTORY}"
    COMMENT "Copying resources from ${RESOURCE_DIRECTORY} to ${OUTPUT_DIRECTORY}"
)

add_custom_command(TARGET blocks_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIRECTORY}"
    "${OUTPUT_DIRECTORY}"
    COMMENT "Copying resources from ${RESOURCE_DIRECTORY} to ${OUTPUT_DIRECTORY}"
)

add_custom_command(TARGET blocks_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIRECTORY}"
    "${OUTPUT_DIRECTORY}"
    COMMENT "Copying resources from ${RESOURCE_DIRECTORY} to ${OUTPUT_DIRECTORY}"
)