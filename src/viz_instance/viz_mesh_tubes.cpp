#include "viz_mesh_tubes.h"

#include <imgui.h>
#include <imgui_helper.h>

#include <shader.h>
#include <logger.h>
#include <glm/gtc/matrix_transform.hpp>
#include <viz_mesh_shaders.h>

viz::viz_mesh_tubes::viz_mesh_tubes(const std::string &fieldCode) {
    handler = std::make_unique<decltype(handler)::element_type>();

    common::graphics_logger()->debug("Compiling shaders...");

    task_shader = common::loadCompileShaderString(gl::GL_TASK_SHADER_NV, {viz::common_header, viz::mag_cheb, fieldCode.c_str(), viz::tubes_constants, viz::tubes_task_main}, "tubes_t");
    mesh_shader = common::loadCompileShaderString(gl::GL_MESH_SHADER_NV, {viz::common_header, viz::mag_cheb, fieldCode.c_str(), viz::tubes_constants, viz::curl_func, viz::tubes_mesh_main}, "tubes_m");
    frag_shader = common::loadCompileShaderString(gl::GL_FRAGMENT_SHADER, {viz::tubes_fragment}, "tubes_f");

    program = common::createProgram({task_shader, mesh_shader, frag_shader});

    common::graphics_logger()->debug("Done compiling shaders!");

    generatePoints();

    gl::glCreateBuffers(1, &settings);

    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_BUFFER, settings, -1, "settings");
    }

    gl::glNamedBufferData(settings, sizeof(Settings), &settings_state, gl::GL_STREAM_DRAW);
}

viz::viz_mesh_tubes::~viz_mesh_tubes() {
    gl::glDeleteBuffers(1, &settings);
}

void viz::viz_mesh_tubes::recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) {
    common::graphics_logger()->debug("Recompiling shaders...");

    const auto shader1 = common::loadCompileShaderString(gl::GL_TASK_SHADER_NV, {viz::common_header, viz::mag_cheb, codes[0].get().c_str(), viz::tubes_constants, viz::tubes_task_main}, "tubes_t");
    const auto shader2 = common::loadCompileShaderString(gl::GL_MESH_SHADER_NV, {viz::common_header, viz::mag_cheb, codes[0].get().c_str(), viz::tubes_constants, viz::curl_func, viz::tubes_mesh_main}, "tubes_m");

    const auto program2 = common::createProgram({shader1, shader2, frag_shader});

    if (program != 0) {
        gl::glDeleteProgram(program);
    }

    program = program2;

    gl::glDeleteShader(shader1);
    gl::glDeleteShader(shader2);

    common::graphics_logger()->debug("Done recompiling shaders!");
}

void viz::viz_mesh_tubes::drawFrame(std::size_t frame, const glm::mat4 &mvp) {
    ImGui::SetNextWindowSize(ImVec2(425,150), ImGuiCond_FirstUseEver);
    ImGui::Begin("Tube settings");

    static int instances = 580;
    static int segments = 61;
    static int circle_lod = 8;
    static float radius = 10.0f;
    static float step = 10.0f;

    ImGui::SliderInt("Instances", &instances, 1, 2000); ImGui::SameLine(); HelpMarker("How many tubes are rendered");

    if (ImGui::SliderInt("Segments", &segments, 1, 255)) {
        if (segments % 2 == 0) {
            segments++;
        }
    } ImGui::SameLine(); HelpMarker("How many segments each tube has");

    ImGui::SliderInt("Circle LOD", &circle_lod, 3, 16); ImGui::SameLine(); HelpMarker("How many vertices a cross section of a tube has");
    ImGui::SliderFloat("Radius", &radius, 1.0f, 500.0f); ImGui::SameLine(); HelpMarker("The maximum radius of a tube");
    ImGui::SliderFloat("Step", &step, 0.1f, 100.0f); ImGui::SameLine(); HelpMarker("Step size used to generate each tube");

    ImGui::End();

    state.MVP = mvp;

    settings_state.segmentsCount = segments;
    settings_state.circleVertices = circle_lod;
    settings_state.radius = radius;
    settings_state.stepSize = step;

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
