#include "context_egl.h"

#include "config.h"

#include <glbinding-aux/ContextInfo.h>
#include <imgui.h>
#include <set>

std::string eglErrorString(EGLint error) {
	switch (error) {
		case EGL_SUCCESS:
			return "EGL_SUCCESS";
		case EGL_NOT_INITIALIZED:
			return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:
			return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:
			return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:
			return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONTEXT:
			return "EGL_BAD_CONTEXT";
		case EGL_BAD_CONFIG:
			return "EGL_BAD_CONFIG";
		case EGL_BAD_CURRENT_SURFACE:
			return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY:
			return "EGL_BAD_DISPLAY";
		case EGL_BAD_SURFACE:
			return "EGL_BAD_SURFACE";
		case EGL_BAD_MATCH:
			return "EGL_BAD_MATCH";
		case EGL_BAD_PARAMETER:
			return "EGL_BAD_PARAMETER";
		case EGL_BAD_NATIVE_PIXMAP:
			return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:
			return "EGL_BAD_NATIVE_WINDOW";
		case EGL_CONTEXT_LOST:
			return "EGL_CONTEXT_LOST";
		default:
			return "Unknown error";
	}
}

void EGLAPIENTRY error_callback(EGLenum error, const char *command, EGLint messageType, EGLLabelKHR threadLabel, EGLLabelKHR objectLabel, const char *message) {
	switch (messageType) {
		case EGL_DEBUG_MSG_CRITICAL_KHR:
		case EGL_DEBUG_MSG_ERROR_KHR:
			spdlog::get("egl")->error("{} {}: {}", eglErrorString(error), command, eglErrorString(error), message);
			break;
		case EGL_DEBUG_MSG_WARN_KHR:
			spdlog::get("egl")->warn("{} {}: {}", eglErrorString(error), command, eglErrorString(error), message);
			break;
		case EGL_DEBUG_MSG_INFO_KHR:
			spdlog::get("egl")->warn("{} {}: {}", eglErrorString(error), command, eglErrorString(error), message);
			break;
		default:
			break;
	}
}

common::context_egl::context_egl() : context(nullptr), surface(nullptr) {
	if (spdlog::get("egl") == nullptr) {
		logger = spdlog::stdout_color_mt("egl");

		if constexpr (common::isDebug) {
			logger->set_level(spdlog::level::debug);
		}
	} else {
	    logger = spdlog::get("egl");
	}

	if constexpr (common::isDebug) {
		const auto clientExts = std::string(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
		std::istringstream iss(clientExts);

		std::set<std::string> extensions(std::istream_iterator<std::string>{iss},
										 std::istream_iterator<std::string>());

		logger->debug("EGL extensions:");

		for (const auto &ext : extensions) {
			logger->debug("\t{}", ext);
		}

		auto dbgPos = extensions.find("EGL_KHR_debug");

		if (dbgPos != extensions.end()) {
			logger->debug("EGL Debugging supported");
			eglDebugMessageControlKHR = reinterpret_cast<PFNEGLDEBUGMESSAGECONTROLKHRPROC>(eglGetProcAddress("eglDebugMessageControlKHR"));

			const EGLAttrib attribs[] = {
					EGL_DEBUG_MSG_CRITICAL_KHR,
					EGL_DEBUG_MSG_ERROR_KHR,
					EGL_DEBUG_MSG_WARN_KHR,
					EGL_DEBUG_MSG_INFO_KHR,
			};

			eglDebugMessageControlKHR(error_callback, nullptr);
		}
	}

	eglQueryDevicesEXT = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
	eglQueryDeviceAttribEXT = reinterpret_cast<PFNEGLQUERYDEVICEATTRIBEXTPROC>(eglGetProcAddress("eglQueryDeviceAttribEXT"));
	eglQueryDeviceStringEXT = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(eglGetProcAddress("eglQueryDeviceStringEXT"));
	eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));

	if (eglQueryDevicesEXT == nullptr || eglQueryDeviceAttribEXT == nullptr || eglQueryDeviceStringEXT == nullptr) {
		logger->error("QueryDevice function pointers not available");
		exit(EXIT_FAILURE);
	}

	if (eglGetPlatformDisplayEXT == nullptr) {
		logger->error("PlatformDisplay pointer not available");
		exit(EXIT_FAILURE);
	}

	EGLDeviceEXT devices[MAX_DEVICES];
	EGLint numDevices;

	if (!eglQueryDevicesEXT(MAX_DEVICES, devices, &numDevices)) {
		logger->error("Failed to query devices");
		exit(EXIT_FAILURE);
	}

	if (numDevices == 0) {
		logger->error("No devices");
		exit(EXIT_FAILURE);
	}

	if constexpr (common::isDebug) {
		logger->debug("Found {} device(s)", numDevices);
	}

	for (int devIdx = 0; devIdx < numDevices; devIdx++) {
		if constexpr (common::isDebug) {
			logger->debug("Device {}", devIdx);
		}

		const auto devExts = eglQueryDeviceStringEXT(devices[devIdx], EGL_EXTENSIONS);
		const auto renderer = eglQueryDeviceStringEXT(devices[devIdx], EGL_RENDERER_EXT);

		if constexpr (common::isDebug) {
			logger->debug("\tName: {}", renderer == nullptr ? "nullptr" : renderer);
			logger->debug("\tExtensions: {}", devExts);
		}
	}


	display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[common::deviceID], nullptr);

	EGLint majorVersion, minorVersion;

	if (!eglInitialize(display, &majorVersion, &minorVersion)) {
		logger->error("No devices");
		exit(EXIT_FAILURE);
	}

	if constexpr (common::isDebug) {
		logger->debug("EGL Version {}.{}", majorVersion, minorVersion);
		logger->debug("Vendor: {}", eglQueryString(display, EGL_VENDOR));
		logger->debug("Client APIs: {}", eglQueryString(display, EGL_CLIENT_APIS));
	}

}

common::context_egl::~context_egl() {
    if (context != nullptr) {
        eglDestroyContext(display, context);
    }

    if (surface != nullptr) {
        eglDestroySurface(display, surface);
    }

	eglTerminate(display);
}

void common::context_egl::create() {
	const int attribs[] = {
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_DEPTH_SIZE, 24,
			EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
			EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
			EGL_NONE};

	EGLint numConfigs;

	if (!eglChooseConfig(display, attribs, nullptr, 0, &numConfigs) || numConfigs < 1) {
		logger->error("No compatibile display configurations");
		exit(EXIT_FAILURE);
	}

	std::vector<EGLConfig> configs(numConfigs);

	if (!eglChooseConfig(display, attribs, configs.data(), numConfigs, &numConfigs) || numConfigs < 1) {
		logger->error("Cannot retrieve compatibile display configurations");
		exit(EXIT_FAILURE);
	}

	const int surfaceAttribs[] = {
			EGL_WIDTH, common::windowWidth,
			EGL_HEIGHT, common::windowHeight,
			EGL_NONE};

	if ((surface = eglCreatePbufferSurface(display, configs[0], surfaceAttribs)) == nullptr) {
		logger->error("Cannot create surface");
		exit(EXIT_FAILURE);
	}

	if (!eglBindAPI(EGL_OPENGL_API)) {
		logger->error("Cannot bind OpenGL API");
		exit(EXIT_FAILURE);
	}

	const int contextAttribs[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
			EGL_CONTEXT_MINOR_VERSION_KHR, 6,
			EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
			EGL_CONTEXT_FLAGS_KHR, common::isDebug ? EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0,
			EGL_NONE};

	if ((context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, contextAttribs)) == nullptr) {
		logger->error("Cannot create OpenGL context");
		exit(EXIT_FAILURE);
	}

	if (!eglMakeCurrent(display, surface, surface, context)) {
		logger->error("Cannot make OpenGL context current");
		exit(EXIT_FAILURE);
	}
}

bool common::context_egl::shouldClose() {
	return false;
}

void common::context_egl::pollEvents() {
}

glbinding::GetProcAddress common::context_egl::getProcAddressPtr() {
	return eglGetProcAddress;
}

void common::context_egl::swapBuffers() {
	eglSwapBuffers(display, surface);
}

void common::context_egl::imguiImplInit() {
    ImGui::StyleColorsDark();
}

void common::context_egl::imguiImplShutdown() {

}

void common::context_egl::imguiImplNewFrame() {
    ImGuiIO& io = ImGui::GetIO();

    // Build font atlas
    unsigned char* tex_pixels = NULL;
    int tex_w, tex_h;
    io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

    io.DisplaySize = ImVec2(common::windowWidth, common::windowHeight);

}

void common::context_egl::imguiImplRender() {

}
