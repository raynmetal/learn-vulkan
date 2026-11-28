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

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
find_package(glm 1.0 REQUIRED)
find_package(Vulkan)
find_package(assimp 6.0 REQUIRED)

target_link_libraries(hello-triangle PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3
    glm::glm
    assimp::assimp
)

set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

