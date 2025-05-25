cmake_minimum_required(VERSION 3.15)

project(MineC_Client)



add_executable(minec_client 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minec_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application_window/application_window.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings/settings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/resources.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/renderer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/rendering_thread.c
)

target_include_directories(minec_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/dependencies/stb_image/include
)

target_link_libraries(minec_client 
    PRIVATE window pixelchar utility networking
)

target_compile_definitions(minec_client PRIVATE MINEC_CLIENT_DEBUG=1)

add_library(minec_client_renderer_backend SHARED 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/backend.c
)

target_include_directories(minec_client_renderer_backend PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(minec_client_renderer_backend 
    PRIVATE window pixelchar utility
)

target_compile_definitions(minec_client_renderer_backend PRIVATE MINEC_CLIENT_DEBUG=1)

add_executable(minec_client_renderer_backend_dummy ${CMAKE_CURRENT_SOURCE_DIR}/src/dummy_main.c)
target_link_libraries(minec_client_renderer_backend_dummy PRIVATE minec_client_renderer_backend )
target_link_libraries(minec_client PRIVATE minec_client_renderer_backend )


target_sources(minec_client_renderer_backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/vulkan/base.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/vulkan/device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/vulkan/pipelines_resources.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/vulkan/render.c
)

target_compile_definitions(minec_client_renderer_backend
    PRIVATE _MINEC_CLIENT_RENDERER_BACKEND_VULKAN=1
)

if(WIN32)
    message(STATUS "Compiling for Windows")

    target_compile_definitions(minec_client
        PRIVATE MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME="minec_client_renderer_backend.dll"
    )
    target_compile_definitions(minec_client_renderer_backend
        PRIVATE MINEC_CLIENT_VULKAN_LIBRARY_NAME="vulkan-1.dll"
    )

elseif(UNIX AND NOT APPLE)
    message(STATUS "Compiling for Linux")

    target_link_libraries(minec_client 
        PRIVATE pthread m
    )

    target_compile_definitions(minec_client
        PRIVATE MINEC_CLIENT_RENDERER_BACKEND_LIBRARY_NAME="libminec_client_renderer_backend.so"
    )
    target_compile_definitions(minec_client_renderer_backend
        PRIVATE MINEC_CLIENT_VULKAN_LIBRARY_NAME="libvulkan.so"
    )

else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

add_custom_target(minec_client_copy_runtime_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/runtime_files" "${CMAKE_CURRENT_BINARY_DIR}/runtime_files"
    COMMENT "Copying runtime files..."
)
add_custom_target(minec_client_copy_renderer_backend ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:minec_client_renderer_backend> "${CMAKE_CURRENT_BINARY_DIR}/runtime_files"
    COMMENT "Copying renderer backend ..."
)

add_dependencies(minec_client minec_client_copy_runtime_files minec_client_copy_renderer_backend)
add_dependencies(minec_client_renderer_backend minec_client_copy_runtime_files)