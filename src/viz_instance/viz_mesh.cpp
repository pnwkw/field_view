#include "viz_mesh.h"

#include <random>
#include <span>

#include <glm/gtc/constants.hpp>

viz::viz_mesh::viz_mesh() : program(0) {
    gl::glCreateBuffers(1, &vertex_buffer);
    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, vertex_buffer, -1, "Vertex buffer");
    }

    gl::glCreateBuffers(1, &ubo);
    if (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, ubo, -1, "ubo");
    }

    gl::glNamedBufferData(ubo, sizeof(State), &state, gl::GL_STREAM_DRAW);
}

viz::viz_mesh::~viz_mesh() {
    gl::glDeleteShader(task_shader);
    gl::glDeleteShader(mesh_shader);
    gl::glDeleteShader(frag_shader);

    gl::glDeleteProgram(program);

    gl::glDeleteBuffers(1, &ubo);
    gl::glDeleteBuffers(1, &vertex_buffer);
}

void viz::viz_mesh::generatePoints() {
    points.clear();

    std::mt19937 rng(common::randomSeed);

    std::uniform_int_distribution<int> generator_x(mag_field::detector_min.x, mag_field::detector_max.x);
    std::uniform_int_distribution<int> generator_y(mag_field::detector_min.y, mag_field::detector_max.y);
    std::uniform_int_distribution<int> generator_z(mag_field::detector_min.z, mag_field::detector_max.z);

    for (std::size_t i = 0; i < common::samples; ++i) {
        points.emplace_back(generator_x(rng), generator_y(rng), generator_z(rng), 1);
    }

    //Replace random data with a couple of test points
    if constexpr (common::isDebug) {
        points[0] = glm::vec4(0, 0, 0, 1);
        points[1] = glm::vec4(0, 0, 550, 1);
        points[2] = glm::vec4(0, 0, -550, 1);
        points[3] = glm::vec4(-200, 0, 0, 1);
        points[4] = glm::vec4(200, 0, 0, 1);
        points[5] = glm::vec4(0, -200, 0, 1);
        points[6] = glm::vec4(0, 200, 0, 1);
    }

    gl::glNamedBufferData(vertex_buffer, std::span(points).size_bytes(), points.data(), gl::GL_STATIC_DRAW);
}