#ifndef COMMON_CONTEXT_EGL_H
#define COMMON_CONTEXT_EGL_H

#include "context_base.h"
#include <memory>

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace common {
	class context_egl final : public context_base {
	private:
		constexpr static auto MAX_DEVICES = 32;

		PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT;
		PFNEGLQUERYDEVICEATTRIBEXTPROC eglQueryDeviceAttribEXT;
		PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT;
		PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;

		PFNEGLDEBUGMESSAGECONTROLKHRPROC eglDebugMessageControlKHR;

		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;

	public:
		context_egl();

		~context_egl() final;

		void create() final;

		bool shouldClose() final;

		void pollEvents() final;

		glbinding::GetProcAddress getProcAddressPtr() final;

		void swapBuffers() final;

        void imguiImplInit() final;
        void imguiImplShutdown() final;
        void imguiImplNewFrame() final;
        void imguiImplRender() final;
	};
}// namespace common

#endif //COMMON_CONTEXT_EGL_H
