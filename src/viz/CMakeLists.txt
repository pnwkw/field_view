add_library(viz STATIC)
add_library(viz::viz ALIAS viz)

target_include_directories(viz PUBLIC include)

target_sources(viz PRIVATE viz_window.cpp)

target_sources(viz PRIVATE glsl.cpp)

target_sources(viz PRIVATE save_frame.cpp)

target_link_libraries(viz glbinding::glbinding glbinding::glbinding-aux spdlog::spdlog bitmap_image::bitmap_image lodepng::lodepng imgui::imgui imgui::imguizmo)

if (DEFINED USE_GLFW_CONTEXT)
    target_link_libraries(viz imgui::imgui_glfw)
    target_compile_definitions(viz PRIVATE USE_GLFW_CONTEXT)
endif (DEFINED USE_GLFW_CONTEXT)

target_link_libraries(viz common::common glm::glm mag_field::mag_field viz::viz_instance)
