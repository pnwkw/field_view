cmake_minimum_required(VERSION 3.16)

project(field_view VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(USE_GLFW_CONTEXT true)
#set(USE_EGL_CONTEXT true)

add_subdirectory(dependencies)
add_subdirectory(src)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})