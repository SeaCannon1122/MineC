cmake_minimum_required(VERSION 3.15)

project(MineC_Client)

set(MINEC_CLIENT_SOURCE_FILES

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minec_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/application.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/application_window.c

    ${CMAKE_SOURCE_DIR}/shared/general/src/key_value.c
    ${CMAKE_SOURCE_DIR}/shared/general/src/resource_manager.c
    ${CMAKE_SOURCE_DIR}/shared/general/src/string_allocator.c
    ${CMAKE_SOURCE_DIR}/shared/general/src/platformlib/platform/platform.c
    ${CMAKE_SOURCE_DIR}/shared/general/src/platformlib/networking/networking.c
)

add_executable(minec_client ${MINEC_CLIENT_SOURCE_FILES})

target_include_directories(minec_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/shared/pixelchar/include
        ${CMAKE_SOURCE_DIR}/shared/general/src
        ${CMAKE_SOURCE_DIR}/dependencies/stb_image/include
)

if(WIN32)
    set_target_properties(minec_client 
        PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

elseif(UNIX)
    find_package(X11 REQUIRED)

    target_include_directories(minec_client
        PRIVATE ${X11_INCLUDE_DIR}
    )
    
    target_link_libraries(minec_client 
        PRIVATE 
            X11 
            pthread 
            m
    )
   
endif()

find_package(Vulkan)
if (Vulkan_FOUND)

    set(MINEC_CLIENT_RENDERER_BACKEND_VULKAN_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/vulkan/vulkan_backend.c
    )
    add_library(minec_client_renderer_backend_vulkan ${MINEC_CLIENT_RENDERER_BACKEND_VULKAN_SOURCE_FILES})

    target_compile_definitions(minec_client_renderer_backend_vulkan
        PUBLIC _MINEC_CLIENT_RENDERER_BACKEND_VULKAN=1
    )
    target_link_libraries(minec_client_renderer_backend_vulkan 
        PUBLIC pixelchar
    )

    target_link_libraries(minec_client PUBLIC pixelchar)

endif()



add_custom_command(TARGET minec_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/runtime_files"
    "${CMAKE_CURRENT_BINARY_DIR}/runtime_files"
)