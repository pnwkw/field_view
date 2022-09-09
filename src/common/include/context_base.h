#ifndef COMMON_CONTEXT_H
#define COMMON_CONTEXT_H

#include <glbinding/gl46core/gl.h>
#include <glbinding/glbinding.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>

namespace common {
	class context_base {
	protected:
		std::shared_ptr<spdlog::logger> logger;

	public:
		using procPtr_t = std::add_pointer_t<void()>;
		using getProcPtr_t = std::add_pointer<procPtr_t(const char *)>;

		virtual ~context_base() = default;

		virtual void create() = 0;

		virtual bool shouldClose() = 0;

		virtual void pollEvents() = 0;

		virtual glbinding::GetProcAddress getProcAddressPtr() = 0;

		virtual void swapBuffers() = 0;

        virtual void imguiImplInit() = 0;
        virtual void imguiImplShutdown() = 0;
        virtual void imguiImplNewFrame() = 0;
        virtual void imguiImplRender() = 0;
	};

}// namespace common

#endif //COMMON_CONTEXT_H
