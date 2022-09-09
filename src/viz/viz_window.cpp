#include "viz_window.h"

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>

#include <imgui.h>
#include <imGuIZMOquat.h>

#include <config.h>
#include <logger.h>
#include <imgui_helper.h>

#include <viz_mesh_lines.h>
#include <viz_mesh_ribbons.h>
#include <viz_mesh_tubes.h>
#include <viz_mesh_surface.h>
#include <save_frame.h>
#include <viz_mesh_shaders.h>

void GL_APIENTRY GLerror_callback(gl::GLenum source, gl::GLenum type, gl::GLuint id, gl::GLenum severity, gl::GLsizei length, const gl::GLchar *message, const void *userParam) {
	if (severity != gl::GL_DEBUG_SEVERITY_NOTIFICATION) {
		switch (type) {
			case gl::GLenum::GL_DEBUG_TYPE_ERROR:
				common::graphics_logger()->error("[{}] {}", glbinding::aux::Meta::getString(severity), message);
				break;
			default:
				common::graphics_logger()->info("[{}] {}", glbinding::aux::Meta::getString(severity), message);
				break;
		}
	}
}

viz::viz_window::viz_window(common::context_base &context) : context(context), frame(0) {
	context.create();

	glbinding::initialize(context.getProcAddressPtr(), true);

	if constexpr (common::isDebug) {
		if (glbinding::aux::ContextInfo::supported({gl::GLextension::GL_KHR_debug})) {
			gl::ContextFlagMask ctxFlag;
			gl::glGetIntegerv(gl::GL_CONTEXT_FLAGS, reinterpret_cast<gl::GLint *>(&ctxFlag));
			if ((ctxFlag & gl::GL_CONTEXT_FLAG_DEBUG_BIT) == gl::GL_CONTEXT_FLAG_DEBUG_BIT) {
				gl::glDebugMessageCallback(GLerror_callback, nullptr);
				gl::glEnable(gl::GL_DEBUG_OUTPUT);
				gl::glEnable(gl::GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}
		} else {
			common::graphics_logger()->warn("{} not supported. OpenGL logging disabled.", glbinding::aux::Meta::getString(gl::GLextension::GL_KHR_debug));
		}
	}

	common::graphics_logger()->info("Selected GPU: {}", common::deviceID);
	common::graphics_logger()->info("Current GPU: {}", glbinding::aux::ContextInfo::renderer());
	common::graphics_logger()->info("Vendor: {}", glbinding::aux::ContextInfo::vendor());
	const auto ver = glbinding::aux::ContextInfo::version();
	common::graphics_logger()->info("OpenGL Version: {}.{}", ver.majorVersion(), ver.minorVersion());

	if constexpr (common::isDebug) {
		common::graphics_logger()->debug("OpenGL Extensions:");
		const auto extensions = glbinding::aux::ContextInfo::extensions();
		for (const auto &ext : extensions) {
			common::graphics_logger()->debug("\t{}", glbinding::aux::Meta::getString(ext));
		}
	}

    const auto meshShaderSupported = glbinding::aux::ContextInfo::supported({gl::GLextension::GL_NV_mesh_shader});
    common::graphics_logger()->info("Mesh shaders are {}supported", meshShaderSupported ? "" : "not ");

    if (!meshShaderSupported) {
        throw std::runtime_error("Unsupported required features!");
    }

    if constexpr (common::isDebug) {
        gl::GLint task_x, task_y, task_z;
        gl::glGetIntegeri_v(gl::GL_MAX_TASK_WORK_GROUP_SIZE_NV, 0, &task_x);
        gl::glGetIntegeri_v(gl::GL_MAX_TASK_WORK_GROUP_SIZE_NV, 1, &task_y);
        gl::glGetIntegeri_v(gl::GL_MAX_TASK_WORK_GROUP_SIZE_NV, 2, &task_z);
        common::graphics_logger()->info("GL_MAX_TASK_WORK_GROUP_SIZE_NV {} {} {}", task_x, task_y, task_z);

        gl::GLint max_mesh_tasks;
        gl::glGetIntegerv(gl::GL_MAX_DRAW_MESH_TASKS_COUNT_NV, &max_mesh_tasks);
        common::graphics_logger()->info("GL_MAX_DRAW_MESH_TASKS_COUNT_NV {}", max_mesh_tasks);

        gl::GLint mesh_x, mesh_y, mesh_z;
        gl::glGetIntegeri_v(gl::GL_MAX_MESH_WORK_GROUP_SIZE_NV, 0, &mesh_x);
        gl::glGetIntegeri_v(gl::GL_MAX_MESH_WORK_GROUP_SIZE_NV, 1, &mesh_y);
        gl::glGetIntegeri_v(gl::GL_MAX_MESH_WORK_GROUP_SIZE_NV, 2, &mesh_z);
        common::graphics_logger()->info("GL_MAX_MESH_WORK_GROUP_SIZE_NV {} {} {}", mesh_x, mesh_y, mesh_z);

        gl::GLint max_mesh_output_vertices;
        gl::glGetIntegerv(gl::GL_MAX_MESH_OUTPUT_VERTICES_NV, &max_mesh_output_vertices);
        common::graphics_logger()->info("GL_MAX_MESH_OUTPUT_VERTICES_NV {}", max_mesh_output_vertices);

        gl::GLint max_mesh_output_primitives;
        gl::glGetIntegerv(gl::GL_MAX_MESH_OUTPUT_PRIMITIVES_NV, &max_mesh_output_primitives);
        common::graphics_logger()->info("GL_MAX_MESH_OUTPUT_PRIMITIVES_NV {}", max_mesh_output_primitives);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    context.imguiImplInit();

    field_editor = std::make_unique<TextEditor>();
    const auto lang = TextEditor::LanguageDefinition::GLSL();
    field_editor->SetLanguageDefinition(lang);
    field_editor->SetPalette(TextEditor::GetDarkPalette());

    field_code = viz::field_func_template;
    field_editor->SetText(field_code);

    curiosity_editor = std::make_unique<TextEditor>();
    curiosity_editor->SetLanguageDefinition(lang);
    curiosity_editor->SetPalette(TextEditor::GetDarkPalette());

    curiosity_code = viz::curiosity_func_template;
    curiosity_editor->SetText(curiosity_code);

    gl::glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    gl::glClearDepth(1.0f);
    gl::glDepthFunc(gl::GL_LEQUAL);
}

viz::viz_window::~viz_window() {
    context.imguiImplShutdown();
    ImGui::DestroyContext();
	gl::glFinish();
}

void viz::viz_window::startFrame() {
	context.pollEvents();
    context.imguiImplNewFrame();
    ImGui::NewFrame();
}

void viz::viz_window::drawFrame() {
    static int impl_selection = 0;
    static bool depth = false;
    static bool show_metrics = false;
    static bool show_field_editor = false;
    static bool show_curiosity_editor = false;
    static bool rotate = false;
    static int style = 0;
    static quat qRot = quat(1.f, 0.f, 0.f, 0.f);

    gl::glViewport(0, 0, common::windowWidth, common::windowHeight);
    gl::glClearColor(backgrounds[style].r, backgrounds[style].g, backgrounds[style].b, 1.0f);

    const auto clearBits = depth ? (gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) : (gl::GL_COLOR_BUFFER_BIT);
    gl::glClear(clearBits);

    ImGui::SetNextWindowSize(ImVec2(205, 350), ImGuiCond_FirstUseEver);
    ImGui::Begin("Main");

    ImGui::Text("Rendering");

    const auto b1 = ImGui::RadioButton("Lines", &impl_selection, 0);
    const auto b2 = ImGui::RadioButton("Ribbons", &impl_selection, 1);
    const auto b3 = ImGui::RadioButton("Tubes", &impl_selection, 2);
    const auto b4 = ImGui::RadioButton("Surface", &impl_selection, 3);

    ImGui::Checkbox("Edit field code", &show_field_editor); ImGui::SameLine(); HelpMarker("Show field code editor");

    if (impl_selection == 3) {
        ImGui::Checkbox("Edit curiosity code", &show_curiosity_editor); ImGui::SameLine(); HelpMarker("Show curiosity code editor");
    }

    ImGui::Separator();

    ImGui::Text("Visual");

    if (ImGui::Combo("Style", &style, "Dark\0Light\0Classic\0")) {
        switch (style) {
            case 0:
                ImGui::StyleColorsDark();
                field_editor->SetPalette(TextEditor::GetDarkPalette());
                curiosity_editor->SetPalette(TextEditor::GetDarkPalette());
                break;
            case 1:
                ImGui::StyleColorsLight();
                field_editor->SetPalette(TextEditor::GetLightPalette());
                curiosity_editor->SetPalette(TextEditor::GetLightPalette());
                break;
            case 2:
                ImGui::StyleColorsClassic();
                field_editor->SetPalette(TextEditor::GetRetroBluePalette());
                curiosity_editor->SetPalette(TextEditor::GetRetroBluePalette());
                break;
        }
    }

    if (ImGui::Checkbox("Depth test", &depth)) {
        if (depth) {
            gl::glEnable(gl::GL_DEPTH_TEST);
        } else {
            gl::glDisable(gl::GL_DEPTH_TEST);
        }
    } ImGui::SameLine(); HelpMarker("Enable depth test, which causes the geometry to be shown with correct occlusion");

    ImGui::Checkbox("Auto rotate camera", &rotate); ImGui::SameLine(); HelpMarker("Slowly rotate the camera around the origin point");

    ImGui::Separator();

    ImGui::Text("Misc");

    ImGui::Checkbox("Open metrics", &show_metrics); ImGui::SameLine(); HelpMarker("Enable keyboard controls.");

    bool capture = ImGui::Button("Capture frame as PNG");

    ImGui::End();

    ImGui::Begin("Orientation");

    ImGui::gizmo3D("", qRot, 200, imguiGizmo::mode3Axes | imguiGizmo::cubeAtOrigin);

    if (ImGui::Button("Reset orientation")) {
        qRot = quat(1.f, 0.f, 0.f, 0.f);
    }

    ImGui::End();

    if (show_field_editor) {
        ImGui::SetNextWindowSize(ImVec2(250, 128), ImGuiCond_FirstUseEver);
        ImGui::Begin("Field code", &show_field_editor, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Code")) {
                const auto m1 = ImGui::MenuItem("Save & Apply", "Ctrl-1");
                const auto m2 = ImGui::MenuItem("Reset & Apply", "Ctrl-2");

                if (m1) {
                    field_code = field_editor->GetText();
                }

                if (m2) {
                    field_code = viz::field_func_template;
                    field_editor->SetText(field_code);
                }

                if ((m1 || m2) && impl) {
                    try {
                        impl->recompileShaders({field_code, curiosity_code});
                        field_editor->SetErrorMarkers({});
                    } catch (std::runtime_error &e) {
                        TextEditor::ErrorMarkers markers;
                        markers.insert(std::make_pair<int, std::string>(1, e.what()));
                        field_editor->SetErrorMarkers(markers);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        field_editor->Render("Field code");
        ImGui::End();
    }

    if (show_curiosity_editor && impl_selection == 3) {
        ImGui::SetNextWindowSize(ImVec2(300, 128), ImGuiCond_FirstUseEver);
        ImGui::Begin("Curiosity code", &show_curiosity_editor, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Code")) {
                const auto m1 = ImGui::MenuItem("Save & Apply", "Ctrl-1");
                const auto m2 = ImGui::MenuItem("Reset & Apply", "Ctrl-2");

                if (m1) {
                    curiosity_code = curiosity_editor->GetText();
                }

                if (m2) {
                    curiosity_code = viz::curiosity_func_template;
                    curiosity_editor->SetText(curiosity_code);
                }

                if ((m1 || m2) && impl) {
                    try {
                        impl->recompileShaders({field_code, curiosity_code});
                        curiosity_editor->SetErrorMarkers({});
                    } catch (std::runtime_error &e) {
                        TextEditor::ErrorMarkers markers;
                        markers.insert(std::make_pair<int, std::string>(1, e.what()));
                        curiosity_editor->SetErrorMarkers(markers);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        curiosity_editor->Render("Curiosity code");
        ImGui::End();
    }

    if (show_metrics) {
        ImGui::ShowMetricsWindow(&show_metrics);
    }

    if (b1 || b2 || b3 || b4 || !impl) {
        switch (impl_selection) {
            case 0:
                impl = std::make_unique<viz_mesh_lines>(field_code);
                break;
            case 1:
                impl = std::make_unique<viz_mesh_ribbons>(field_code);
                break;
            case 2:
                impl = std::make_unique<viz_mesh_tubes>(field_code);
                break;
            case 3:
                impl = std::make_unique<viz_mesh_surface>(field_code);
                break;
        }
    }

    const auto frac = 100.0f;
    const auto cameraPos = glm::vec3{ glm::cos(frame/frac)*2000.0f, 1000.0f, glm::sin(frame/frac)*2000.0f };

    const auto viewMatrix = glm::lookAt(cameraPos, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
    const auto projectionMatrix = glm::perspective(glm::radians(45.0f), common::windowWidth/static_cast<float>(common::windowHeight), 1.0f, 6000.0f);
    const auto modelMatrix = mat4_cast(qRot);

    impl->drawFrame(frame, projectionMatrix * viewMatrix * modelMatrix);

    if (capture) {
        save_frame::save(fmt::format("images/frame_{}", frame), save_frame::PNG);
    }

    ImGui::Render();
    context.imguiImplRender();

    if (rotate) {
        frame++;
    }
}

void viz::viz_window::endFrame() {
	if constexpr (!common::headless) {
		context.swapBuffers();
	}
}
