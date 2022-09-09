#ifndef COMMON_CONTEXT_GLFW_H
#define COMMON_CONTEXT_GLFW_H

#include "context_base.h"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <memory>

namespace common {
	class context_glfw final : public context_base {
	private:
		using DestroyGlfwWin = struct DestroyGlfwWin {
			void operator()(GLFWwindow *ptr) {
				glfwDestroyWindow(ptr);
			}
		};

		using UniqueGLFWWindow = std::unique_ptr<GLFWwindow, DestroyGlfwWin>;

		UniqueGLFWWindow window{};

	public:
		context_glfw();

		~context_glfw() final;

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

#endif //COMMON_CONTEXT_GLFW_H
