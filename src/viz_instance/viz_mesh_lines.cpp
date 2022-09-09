#include "viz_mesh_lines.h"

#include <imgui.h>
#include <imgui_helper.h>

#include <shader.h>
#include <logger.h>
#include <glm/gtc/matrix_transform.hpp>
#include <viz_mesh_shaders.h>

viz::viz_mesh_lines::viz_mesh_lines(const std::string &fieldCode) {
    handler = std::make_unique<decltype(handler)::element_type>();

    common::graphics_logger()->debug("Compiling shaders...");

    task_shader = common::loadCompileShaderString(gl::GL_TASK_SHADER_NV, {viz::lines_ribbons_task}, "lines_t");
    mesh_shader = common::loadCompileShaderString(gl::GL_MESH_SHADER_NV, {viz::common_header, viz::mag_cheb, fieldCode.c_str(), viz::lines_mesh_main}, "lines_m");
    frag_shader = common::loadCompileShaderString(gl::GL_FRAGMENT_SHADER, {viz::common_fragment}, "lines_f");

    program = common::createProgram({task_shader, mesh_shader, frag_shader});

    common::graphics_logger()->debug("Done compiling shaders!");

    generatePoints();

    gl::glCreateBuffers(1, &settings);

    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, settings, -1, "settings");
    }

    gl::glNamedBufferData(settings, sizeof(Settings), &settings_state, gl::GL_STREAM_DRAW);
}

viz::viz_mesh_lines::~viz_mesh_lines() {
    gl::glDeleteBuffers(1, &settings);
}

void viz::viz_mesh_lines::recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) {
    common::graphics_logger()->debug("Recompiling shaders...");

    const auto shader = common::loadCompileShaderString(gl::GL_MESH_SHADER_NV, {viz::common_header, viz::mag_cheb, codes[0].get().c_str(), viz::lines_mesh_main}, "lines_m");

    const auto program2 = common::createProgram({task_shader, shader, frag_shader});

    if (program != 0) {
        gl::glDeleteProgram(program);
    }

    program = program2;

    gl::glDeleteShader(shader);

    common::graphics_logger()->debug("Done recompiling shaders!");
}

void viz::viz_mesh_lines::drawFrame(std::size_t frame, const glm::mat4 &mvp) {
    ImGui::SetNextWindowSize(ImVec2(320, 110), ImGuiCond_FirstUseEver);
    ImGui::Begin("Lines settings");

    static int instances = 600;
    static int segments = 61;
    static float step = 15.0f;

    ImGui::SliderInt("Instances", &instances, 1, 3000); ImGui::SameLine(); HelpMarker("How many lines are rendered");

    if (ImGui::SliderInt("Segments", &segments, 3, 255)) {
        if (segments % 2 == 0) {
            segments++;
        }
    } ImGui::SameLine(); HelpMarker("How many segments each line has");

    ImGui::SliderFloat("Step", &step, 0.1f, 100.0f); ImGui::SameLine(); HelpMarker("Step size used to generate each line");

    ImGui::End();

    state.MVP = mvp;

    settings_state.verticesCount = segments;
    settings_state.step = step;

    gl::glNamedBufferSubData(ubo, 0, sizeof(State), &state);
    gl::glNamedBufferSubData(settings, 0, sizeof(Settings), &settings_state);

    gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, 0, ubo);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 0, settings);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, handler->getSolSegmentsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 2, handler->getDipSegmentsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 3, handler->getSolParamsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 4, handler->getDipParamsBufferName());
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 5, vertex_buffer);

    gl::glUseProgram(program);

    gl::glDrawMeshTasksNV(0, instances);
}
