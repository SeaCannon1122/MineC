cmake_minimum_required(VERSION 3.15)
project(cwindow)

add_library(cwindow STATIC)
target_include_directories(cwindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (WIN32)
    target_sources(cwindow PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cwindow_impl_windows.c)
    target_compile_definitions(cwindow PUBLIC CWINDOW_IMPLEMENTATION_WINDOWS)

elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)

    target_sources(cwindow ${CMAKE_CURRENT_SOURCE_DIR}/src/cwindow_impl_x11.c)
    target_compile_definitions(cwindow PUBLIC CWINDOW_IMPLEMENTATION_X11)
    target_link_libraries(cwindow PUBLIC ${X11_LIBRARIES})

endif()