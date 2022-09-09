add_library(viz_instance STATIC viz_mesh.cpp)
add_library(viz::viz_instance ALIAS viz_instance)

target_include_directories(viz_instance PUBLIC include)

target_sources(viz_instance PRIVATE viz_mesh_lines.cpp)
target_sources(viz_instance PRIVATE viz_mesh_ribbons.cpp)
target_sources(viz_instance PRIVATE viz_mesh_tubes.cpp)
target_sources(viz_instance PRIVATE viz_mesh_surface.cpp)

target_link_libraries(viz_instance viz::viz imgui::imgui imgui::textedit)
