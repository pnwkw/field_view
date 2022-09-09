#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <config.h>
#include <logger.h>

#ifdef USE_GLFW_CONTEXT
#include <context_glfw.h>
#endif

#ifdef USE_EGL_CONTEXT
#include <context_egl.h>
#endif

#include <viz_window.h>

int main() {
    auto logger_graphics = spdlog::stdout_color_mt("graphics");
    auto logger_field = spdlog::stdout_color_mt("field");

    if constexpr (common::isDebug) {
        common::graphics_logger()->set_level(spdlog::level::debug);
        common::field_logger()->set_level(spdlog::level::debug);
        spdlog::set_level(spdlog::level::debug);
    }

    std::set_terminate([]() -> void {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception &ex) {
            spdlog::error("{}", ex.what());
        }

        std::abort();
    });

    common::configRead("data/config.json");

#ifdef USE_GLFW_CONTEXT
    common::context_glfw context;
#endif

#ifdef USE_EGL_CONTEXT
    common::context_egl context;
#endif

    {
        auto viz_window = viz::viz_window(context);

		do {
			viz_window.startFrame();
			viz_window.drawFrame();
			viz_window.endFrame();
		} while (!viz_window.shouldClose() && !common::headless);

    }

    return EXIT_SUCCESS;
}