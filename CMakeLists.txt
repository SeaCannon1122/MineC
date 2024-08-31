cmake_minimum_required(VERSION 3.15)
project(blocks2)


set(CLIENT_SOURCE_FILES
    src/general/utils.c
    src/client/gui/char_font.c
    src/client/platform.c
    src/client/main.c
    src/client/gui/menu.c

    src/general/resource_manager.c
    src/general/parallel_computing.c
)

set(SERVER_SOURCE_FILES
    src/server/main.c

    src/general/resource_manager.c
    src/general/parallel_computing.c
)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/stb_image)

include_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/inc)
link_directories(${PROJECT_SOURCE_DIR}/dependencies/OpenCL/lib/x64)

add_executable(blocks_client ${CLIENT_SOURCE_FILES} )
add_executable(blocks_server ${SERVER_SOURCE_FILES} )

target_link_libraries(blocks_client OpenCL)
target_link_libraries(blocks_server OpenCL)

if(WIN32)
    set_target_properties(blocks_client PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(blocks_client X11 pthread m)
endif()

set(RESOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/resources")
set(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

add_custom_command(TARGET blocks_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIRECTORY}"
    "${OUTPUT_DIRECTORY}"
    COMMENT "Copying resources from ${RESOURCE_DIRECTORY} to ${OUTPUT_DIRECTORY}"
)