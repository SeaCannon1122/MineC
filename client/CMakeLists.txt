cmake_minimum_required(VERSION 3.15)

project(MineC_Client)

message(STATUS "Building MineC_Client")

set(MINEC_CLIENT_CMAKE_BUILD_DEVELOPMENT ON)

# ------------------------ Build Configurations -----------------------

if(MINEC_CLIENT_CMAKE_BUILD_DEVELOPMENT)
    message(STATUS "Build: Development")
    set(MINEC_CLIENT_CMAKE_DYNAMIC_RENDERER ON)
    set(MINEC_CLIENT_CMAKE_DEBUG_LOG ON)

elseif(MINEC_CLIENT_CMAKE_BUILD_DEBUG)
    message(STATUS "Build: Debug")
    set(MINEC_CLIENT_CMAKE_DEBUG_LOG ON)

elseif(MINEC_CLIENT_CMAKE_BUILD_RELEASE)  
    message(STATUS "Build: Release")

else()
    message(FATAL_ERROR "Build: No configuration set")

endif()

add_executable(MineC_Client)
add_library(MineC_Client_Resource_index STATIC)

if(MINEC_CLIENT_CMAKE_DYNAMIC_RENDERER AND WIN32)
    message(STATUS "    Dynamic renderer")

    add_library(MineC_Client_Renderer SHARED)
    set(MINEC_CLIENT_RENDERER_TARGET MineC_Client_Renderer)

    target_compile_definitions(MineC_Client PRIVATE MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE)
    target_compile_definitions(MineC_Client_Renderer PRIVATE MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE)

    add_dependencies(MineC_Client MineC_Client_Renderer)

    add_executable(MineC_Client_Renderer_build_dummy ${CMAKE_CURRENT_SOURCE_DIR}/src/dummy_main.c)
    target_link_libraries(MineC_Client_Renderer_build_dummy PRIVATE MineC_Client_Renderer)

else()
    set(MINEC_CLIENT_RENDERER_TARGET MineC_Client)
endif()

add_custom_target(MineC_Client_copy_data_files
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/MineC_Client_data" "${CMAKE_CURRENT_BINARY_DIR}/MineC_Client_data"
    COMMENT "Copying MineC_Client_data/ ..."
)

add_dependencies(MineC_Client MineC_Client_copy_data_files)
add_dependencies(${MINEC_CLIENT_RENDERER_TARGET} MineC_Client_copy_data_files)

if(MINEC_CLIENT_CMAKE_DEBUG_LOG)
    message(STATUS "    Debug log")

    target_compile_definitions(MineC_Client PRIVATE MINEC_CLIENT_DEBUG_LOG)
    target_compile_definitions(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE MINEC_CLIENT_DEBUG_LOG)
endif()

# ------------------------------ Platform -----------------------------

if(WIN32)
    message(STATUS "Platform: Windows")
    set(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_VULKAN OFF)
    set(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_OPENGL ON)
    set(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_DIRECT3D11 ON)
    set(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_DIRECT3D12 ON)

    target_link_options(MineC_Client PRIVATE "/STACK:8388608" )
    target_sources(MineC_Client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/icon.rc)

    if(MINEC_CLIENT_CMAKE_DYNAMIC_RENDERER)
        target_compile_definitions(MineC_Client PRIVATE MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY_NAME="MineC_Client_Renderer.dll")
        target_link_options(MineC_Client_Renderer_build_dummy PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    endif()

    if(NOT MINEC_CLIENT_CMAKE_DEBUG_LOG)
        target_link_options(MineC_Client PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    endif()

elseif(UNIX AND NOT APPLE)
    message(STATUS "Platform: Linux")
    set(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_VULKAN OFF)
    set(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_OPENGL ON)

    target_link_libraries(MineC_Client PRIVATE pthread m)

else()
    message(FATAL_ERROR "Platform: Unsupported")
endif()

# ---------------------------------------------------------------------
# ------------------------------ Targets ------------------------------
# ---------------------------------------------------------------------

# -------------------- MineC_Client_Resource_index --------------------

target_sources(MineC_Client_Resource_index PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resource_index.c
    ${CMAKE_CURRENT_SOURCE_DIR}/.cerialized_src/resources.c
)

target_include_directories(MineC_Client_Resource_index PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/.cerialized_src
)

target_link_libraries(MineC_Client_Resource_index PRIVATE utility)


# ---------------------------- MineC_Client ---------------------------

target_sources(MineC_Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minec_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string_index/string_index.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application_window/application_window.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings/settings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asset_loader/asset_loader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/renderer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/menu_gui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/pixelfont.c
)

target_include_directories(MineC_Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/dependencies/stb_image/include
)

target_link_libraries(MineC_Client PRIVATE MineC_Client_Resource_index cwindow utility networking stb_image)


# ------------------ ${MINEC_CLIENT_RENDERER_TARGET} ------------------

target_sources(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/renderer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/components.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/backend.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/pixelfont.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/gui_text.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/menu_gui.c
)

target_include_directories(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE MineC_Client_Resource_index cwindow utility stb_image)


# ---------------------------------------------------------------------
# ------------------------- Renderer Backends -------------------------
# ---------------------------------------------------------------------

message(STATUS "Renderer backends:")

# ------------------------------- Vulkan ------------------------------

if(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_VULKAN)
    message(STATUS "  Vulkan")

    target_sources(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/vulkan/backend_vulkan.c
    )

    target_compile_definitions(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE MINEC_CLIENT_RENDERER_BACKEND_VULKAN=1)
endif()

# ------------------------------- OpenGL ------------------------------

if(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_OPENGL)
    message(STATUS "  OpenGL")

    target_sources(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/backend/opengl/backend_opengl.c
    )

    target_compile_definitions(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE MINEC_CLIENT_RENDERER_BACKEND_OPENGL=1)
endif()

# ----------------------------- Direct3d11 ----------------------------

if(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_DIRECT3D11)
    message(STATUS "  Direct3D11")

    target_compile_definitions(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE MINEC_CLIENT_RENDERER_BACKEND_DIRECT3D11=1)
endif()

# ----------------------------- Direct3d12 ----------------------------

if(MINEC_CLIENT_CMAKE_RENDERER_BACKEND_DIRECT3D12)
    message(STATUS "  Direct3D12")

    target_compile_definitions(${MINEC_CLIENT_RENDERER_TARGET} PRIVATE MINEC_CLIENT_RENDERER_BACKEND_DIRECT3D12=1)
endif()