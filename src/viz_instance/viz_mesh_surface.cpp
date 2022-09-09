#include "viz_mesh_surface.h"

#include <imgui.h>
#include <imgui_helper.h>

#include <shader.h>
#include <logger.h>
#include <glm/gtc/type_ptr.hpp>
#include <viz_mesh_shaders.h>

#include <span>

constexpr int LEVELS = 3;

constexpr int level_length(const int level) {
    return 1 << ((LEVELS-level+1) << 1);
}

constexpr int total_length() {
    int sum = 0;
    for (int i = 0; i < LEVELS+1; ++i) {
        sum += level_length(i);
    }

    return sum;
}

viz::viz_mesh_surface::viz_mesh_surface(const std::string &fieldCode) {
    handler = std::make_unique<decltype(handler)::element_type>();

    common::graphics_logger()->debug("Compiling shaders...");

    task_shader = common::loadCompileShaderString(gl::GL_TASK_SHADER_NV, {viz::common_header, viz::mag_cheb, fieldCode.c_str(), viz::curiosity_func_template, viz::surface_constants, viz::surface_task_main}, "surface_t");
    mesh_shader = common::loadCompileShaderString(gl::GL_MESH_SHADER_NV, {viz::common_header, viz::mag_cheb, fieldCode.c_str(), viz::surface_constants, viz::surface_mesh_main}, "surface_m");
    frag_shader = common::loadCompileShaderString(gl::GL_FRAGMENT_SHADER, {viz::common_fragment}, "surface_f");

    program = common::createProgram({task_shader, mesh_shader, frag_shader});

    common::graphics_logger()->debug("Done compiling shaders!");

    const float z1 = -100;
    const float z2 = 100;

    // Generate Points
    points.clear();
    points.emplace_back(mag_field::detector_min.x, mag_field::detector_max.y, z1, 1);
    points.emplace_back(mag_field::detector_min.x, mag_field::detector_min.y, z2, 1);
    points.emplace_back(mag_field::detector_max.x, mag_field::detector_max.y, z1, 1);
    points.emplace_back(mag_field::detector_max.x, mag_field::detector_min.y, z2, 1);

    gl::glNamedBufferData(vertex_buffer, std::span(points).size_bytes(), points.data(), gl::GL_STREAM_DRAW);

    gl::glPointSize(10.0f);

    gl::glCreateBuffers(1, &samples_buffer);
    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, samples_buffer, -1, "Samples buffer");
    }

    gl::glCreateBuffers(1, &keep_levels_buffer);
    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, keep_levels_buffer, -1, "Keep levels buffer");
    }

    gl::glCreateBuffers(1, &mesh_level_buffer);
    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, mesh_level_buffer, -1, "Mesh level buffer");
    }

    gl::glCreateBuffers(1, &mesh_index_buffer);
    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, mesh_index_buffer, -1, "Mesh index buffer");
    }

    gl::glCreateBuffers(1, &settings);

    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, settings, -1, "settings");
    }

    gl::glNamedBufferData(settings, sizeof(Settings), &settings_state, gl::GL_STREAM_DRAW);

    gl::glNamedBufferStorage(samples_buffer, sizeof(float) * level_length(0), nullptr, gl::BufferStorageMask::GL_NONE_BIT);
    gl::glNamedBufferStorage(keep_levels_buffer, sizeof(int) * total_length(), nullptr, gl::BufferStorageMask::GL_NONE_BIT);
    gl::glNamedBufferStorage(mesh_level_buffer, sizeof(int) * level_length(0), nullptr, gl::BufferStorageMask::GL_NONE_BIT);
    gl::glNamedBufferStorage(mesh_index_buffer, sizeof(int) * level_length(0), nullptr, gl::BufferStorageMask::GL_NONE_BIT);
}

viz::viz_mesh_surface::~viz_mesh_surface() {
    gl::glDeleteBuffers(1, &settings);
    gl::glDeleteBuffers(1, &samples_buffer);
    gl::glDeleteBuffers(1, &keep_levels_buffer);
    gl::glDeleteBuffers(1, &mesh_level_buffer);
    gl::glDeleteBuffers(1, &mesh_index_buffer);
}

void viz::viz_mesh_surface::recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) {
    const auto shader1 = common::loadCompileShaderString(gl::GL_TASK_SHADER_NV, {viz::common_header, viz::mag_cheb, codes[0].get().c_str(), codes[1].get().c_str(), viz::surface_constants, viz::surface_task_main}, "surface_t");
    const auto shader2 = common::loadCompileShaderString(gl::GL_MESH_SHADER_NV, {viz::common_header, viz::mag_cheb, codes[0].get().c_str(), viz::surface_constants, viz::surface_mesh_main}, "surface_m");

    const auto program2 = common::createProgram({shader1, shader2, frag_shader});

    if (program != 0) {
        gl::glDeleteProgram(program);
    }

    program = program2;

    gl::glDeleteShader(shader1);
    gl::glDeleteShader(shader2);
}

void viz::viz_mesh_surface::drawFrame(std::size_t frame, const glm::mat4 &mvp) {
    gl::glClearNamedBufferData(keep_levels_buffer, gl::GL_R8UI, gl::GL_RED_INTEGER, gl::GL_UNSIGNED_BYTE, nullptr);
    gl::glClearNamedBufferData(mesh_level_buffer, gl::GL_R8UI, gl::GL_RED_INTEGER, gl::GL_UNSIGNED_BYTE, nullptr);
    gl::glClearNamedBufferData(mesh_index_buffer, gl::GL_R8UI, gl::GL_RED_INTEGER, gl::GL_UNSIGNED_BYTE, nullptr);

    ImGui::SetNextWindowSize(ImVec2(405, 180), ImGuiCond_FirstUseEver);
    ImGui::Begin("Surface settings");

    static float threshold = 1.0f;
    static glm::vec3 offset;

    ImGui::SliderFloat("Threshold", &threshold, 0.1f, 10.0f); ImGui::SameLine(); HelpMarker("Curiosity threshold, samples above this value won't be merged");

    const auto b1 = ImGui::InputFloat3("P1", glm::value_ptr(points[0])); ImGui::SameLine(); HelpMarker("Position of the upper left corner of the surface");
    const auto b2 = ImGui::InputFloat3("P2", glm::value_ptr(points[1])); ImGui::SameLine(); HelpMarker("Position of the lower left corner of the surface");
    const auto b3 = ImGui::InputFloat3("P3", glm::value_ptr(points[2])); ImGui::SameLine(); HelpMarker("Position of the upper right corner of the surface");
    const auto b4 = ImGui::InputFloat3("P4", glm::value_ptr(points[3])); ImGui::SameLine(); HelpMarker("Position of the lower right corner of the surface");

    const auto b5 = ImGui::SliderFloat3("Offset", glm::value_ptr(offset), -2000.f, 700.0f); ImGui::SameLine(); HelpMarker("Position offset for the whole surface");

    ImGui::End();

    if (b1 || b2 || b3 || b4 || b5) {
        static std::array<glm::vec4, 4> data;
        std::copy(points.cbegin(), points.cend(), data.begin());
        for (auto &p : data) {
            p.x += offset.x;
            p.y += offset.y;
            p.z += offset.z;
        }
        gl::glNamedBufferSubData(vertex_buffer, 0, std::span(data).size_bytes(), data.data());
    }

    state.MVP = mvp;
    settings_state.threshold = threshold;

    gl::glNamedBufferSubData(ubo, 0, sizeof(State), &state);
    gl::glNamedBufferSubData(settings, 0, sizeof(Settings), &settings_state);

    gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, 0, ubo);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 0, settings);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, handler->getSolSegmentsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 2, handler->getDipSegmentsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 3, handler->getSolParamsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 4, handler->getDipParamsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 5, vertex_buffer);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 6, samples_buffer);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 7, keep_levels_buffer);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 8, mesh_level_buffer);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 9, mesh_index_buffer);

    gl::glUseProgram(program);

    gl::glDrawMeshTasksNV(0, 1);
}
