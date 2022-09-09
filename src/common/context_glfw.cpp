#include "context_glfw.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "config.h"

void error_callback(int error, const char *description) {
	spdlog::get("glfw")->error("Error {}: {}", error, description);
}

common::context_glfw::context_glfw() {
	if (spdlog::get("glfw") == nullptr) {
		logger = spdlog::stdout_color_mt("glfw");

		if constexpr (common::isDebug) {
			logger->set_level(spdlog::level::debug);
		}
	} else {
        logger = spdlog::get("glfw");
	}

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		logger->error("Can not init GLFW!");
		exit(EXIT_FAILURE);
	}
}

common::context_glfw::~context_glfw() {
	window.reset();
	glfwTerminate();
}

void common::context_glfw::create() {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, common::isDebug);
	glfwWindowHint(GLFW_VISIBLE, !common::headless);

	window = UniqueGLFWWindow(glfwCreateWindow(common::windowWidth, common::windowHeight, "Field View", nullptr, nullptr));

	if (!window) {
		logger->error("Can not open window!");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(0);
}

bool common::context_glfw::shouldClose() {
	return glfwWindowShouldClose(this->window.get());
}

void common::context_glfw::pollEvents() {
	glfwPollEvents();
}

glbinding::GetProcAddress common::context_glfw::getProcAddressPtr() {
	return glfwGetProcAddress;
}

void common::context_glfw::swapBuffers() {
	glfwSwapBuffers(window.get());
}

void common::context_glfw::imguiImplInit() {
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

void common::context_glfw::imguiImplShutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void common::context_glfw::imguiImplNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void common::context_glfw::imguiImplRender() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
