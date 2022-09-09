add_subdirectory(common)
add_subdirectory(mag_field)
add_subdirectory(viz_instance)
add_subdirectory(viz)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} common::common viz::viz spdlog::spdlog)

if (DEFINED USE_GLFW_CONTEXT)
    target_link_libraries(${PROJECT_NAME} common::context_glfw)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLFW_CONTEXT)
endif (DEFINED USE_GLFW_CONTEXT)

if (DEFINED USE_EGL_CONTEXT)
    target_link_libraries(${PROJECT_NAME} common::context_egl)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_EGL_CONTEXT)
endif (DEFINED USE_EGL_CONTEXT)