#ifndef VIZ_VIZ_WINDOW_H
#define VIZ_VIZ_WINDOW_H

#include <memory>
#include <utility>

#include <context_base.h>

#include <viz_mesh.h>
#include <TextEditor.h>

namespace viz {
	class viz_window {
	private:
		common::context_base &context;
        std::unique_ptr<viz::viz_mesh> impl{};

        std::unique_ptr<TextEditor> field_editor;
        std::string field_code;

        std::unique_ptr<TextEditor> curiosity_editor;
        std::string curiosity_code;

        std::array<glm::vec3, 3> backgrounds {
            glm::vec3{0.1f, 0.1f, 0.1f},
            glm::vec3{1.0f, 0.98f, 0.98f},
            glm::vec3{0.25f, 0.29f, 0.33f}
        };

        std::size_t frame;

	public:
		viz_window(common::context_base &context);

		~viz_window();

		bool shouldClose() const { return context.shouldClose(); };

		void startFrame();

		void drawFrame();

		void endFrame();
	};

}// namespace viz

#endif //VIZ_VIZ_WINDOW_H
