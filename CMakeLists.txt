cmake_minimum_required(VERSION 3.15)
project(blocks2)

set(CLIENT_SOURCE_FILES
    src/general/utils.c
    src/client/gui/menu.c
    src/client/game_client_body/game_client.c
    src/client/game_client_body/game_menus.c
    src/client/game_client_body/game_client_renderer.c
    src/client/game_client_body/game_client_simulator.c
    src/client/game_client_body/game_client_networker.c
    src/client/game_client_body/debug.c
    src/client/main.c
    
    src/general/platformlib/platform.c
    src/general/platformlib/parallel_computing.c
    src/general/platformlib/networking.c
    src/general/platformlib/opengl_rendering.c

    src/general/keyvalue.c
    src/general/resource_manager.c
    src/general/resource_loader.c

)

set(SERVER_SOURCE_FILES
    src/server/main.c
    src/server/game_server_body/game_server.c
    src/server/game_server_body/game_simulator.c

    src/general/platformlib/platform.c
    src/general/platformlib/parallel_computing.c
    src/general/platformlib/networking.c

    src/general/utils.c
    src/general/keyvalue.c
    src/general/resource_manager.c
)

set(TEST_SOURCE_FILES
    src/general/utils.c

    src/test/main.c
    src/test/testing.c
    src/client/gui/menu.c

    src/client/gui/pixel_char.c
    src/client/gui/argb_image.c

    src/general/platformlib/platform.c
    src/general/platformlib/parallel_computing.c
    src/general/platformlib/networking.c
    src/general/platformlib/opengl_rendering.c

)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/stb_image/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/inc)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/include)

link_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/lib)
link_directories(${PROJECT_SOURCE_DIR}/dependencies/GLEW/lib/Release/x64)

add_executable(blocks_client ${CLIENT_SOURCE_FILES})
add_executable(blocks_server ${SERVER_SOURCE_FILES})
add_executable(blocks_test ${TEST_SOURCE_FILES})


target_link_libraries(blocks_client OpenCL)
target_link_libraries(blocks_server OpenCL)
target_link_libraries(blocks_test OpenCL)

if(WIN32)

    set_target_properties(blocks_client PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    set_target_properties(blocks_test PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    target_link_libraries(blocks_client opengl32 glew32s)
    target_link_libraries(blocks_test opengl32 glew32s)

elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    
    include_directories(${OPENGL_INCLUDE_DIRS} ${X11_INCLUDE_DIR})

    target_link_libraries(blocks_client ${OPENGL_LIBRARIES} X11 GLEW GLX pthread m)
    target_link_libraries(blocks_test ${OPENGL_LIBRARIES} X11 GLEW GLX pthread m)
    target_link_libraries(blocks_server ${OPENGL_LIBRARIES} X11 pthread m)
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