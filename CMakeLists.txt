cmake_minimum_required(VERSION 3.23)

project(learn-vulkan VERSION 0.1)

add_executable(hello-triangle WIN32)

target_sources(
    hello-triangle
    # Ur application sources
    PRIVATE
        src/hello_triangle_application.cpp
        src/main.cpp

    PRIVATE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/hello_triangle_application.hpp
)

set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

