add_library(mag_field STATIC)
add_library(mag_field::mag_field ALIAS mag_field)

target_include_directories(mag_field PUBLIC include)

target_sources(mag_field PRIVATE mag_cheb.cpp)

target_link_libraries(mag_field glbinding::glbinding glm::glm spdlog::spdlog common::common)
