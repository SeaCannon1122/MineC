cmake_minimum_required(VERSION 3.15)
project(blocks2)

set(CLIENT_SOURCE_FILES
    src/general/utils.c
    src/client/gui/char_font.c
    src/client/gui/menu.c
    src/client/game_client_body/game_client.c
    src/client/game_client_body/game_menus.c
    src/client/main.c
    
    src/general/platformlib/platform.c
    src/general/platformlib/parallel_computing.c
    src/general/platformlib/networking.c

    src/general/keyvalue.c
    src/general/resource_manager.c
    src/general/resource_loader.c

)

set(SERVER_SOURCE_FILES
    src/server/main.c

    src/general/platformlib/platform.c
    src/general/platformlib/parallel_computing.c
    src/general/platformlib/networking.c

    src/general/keyvalue.c
    src/general/resource_manager.c
    src/general/resource_loader.c
)

set(TEST_SOURCE_FILES
    
    src/client/gui/char_font.c
    src/client/gui/menu.c
    src/test/main.c
    
    src/general/platformlib/platform.c
    src/general/platformlib/parallel_computing.c
    src/general/platformlib/networking.c

    src/general/keyvalue.c
    src/general/resource_manager.c
    src/general/resource_loader.c
)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/stb_image)

include_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/inc)
link_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/lib)

add_executable(blocks_client ${CLIENT_SOURCE_FILES} )
add_executable(blocks_server ${SERVER_SOURCE_FILES} )
add_executable(blocks_test ${TEST_SOURCE_FILES} )

target_compile_definitions(blocks_client PRIVATE CLIENT_COMPILATION)
target_compile_definitions(blocks_server PRIVATE SERVER_COMPILATION)

target_link_libraries(blocks_client OpenCL)
target_link_libraries(blocks_server OpenCL)
target_link_libraries(blocks_test OpenCL)

if(WIN32)
set_target_properties(blocks_client PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(blocks_client X11 pthread m)
    target_link_libraries(blocks_server pthread m)
    target_link_libraries(blocks_test pthread m)
endif()

set(RESOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/resources")
set(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

add_custom_command(TARGET blocks_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIRECTORY}"
    "${OUTPUT_DIRECTORY}"
    COMMENT "Copying resources from ${RESOURCE_DIRECTORY} to ${OUTPUT_DIRECTORY}"
)