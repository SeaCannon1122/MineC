cmake_minimum_required(VERSION 3.15)

project(MineC_Client)

set(MINEC_CLIENT_SOURCE_FILES

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minec_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application_window/application_window.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings/settings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/resources.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/renderer.c
)

add_executable(minec_client ${MINEC_CLIENT_SOURCE_FILES})

target_include_directories(minec_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/dependencies/stb_image/include
)

target_link_libraries(minec_client 
    PRIVATE window pixelchar utility networking
)

if(WIN32)


elseif(UNIX)
    target_link_libraries(minec_client 
        PRIVATE pthread m
    )
   
endif()

find_package(Vulkan)
if (Vulkan_FOUND)

    set(MINEC_CLIENT_RENDERER_BACKEND_VULKAN_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/_vulkan/vulkan_backend.c
    )
    add_library(minec_client_renderer_backend_vulkan ${MINEC_CLIENT_RENDERER_BACKEND_VULKAN_SOURCE_FILES})

    target_compile_definitions(minec_client_renderer_backend_vulkan
        PUBLIC _MINEC_CLIENT_RENDERER_BACKEND_VULKAN=1
    )

endif()

add_custom_target(minec_copy_runtime_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/runtime_files"
    "${CMAKE_CURRENT_BINARY_DIR}/runtime_files"
    COMMENT "Copying runtime files..."
)

add_dependencies(minec_client minec_copy_runtime_files)