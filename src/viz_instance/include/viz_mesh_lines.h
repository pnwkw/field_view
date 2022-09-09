#ifndef VIZ_INSTANCE_VIZ_MESH_LINES_H
#define VIZ_INSTANCE_VIZ_MESH_LINES_H

#include <memory>
#include <utility>

#include <viz_mesh.h>

namespace viz {
    class viz_mesh_lines : public viz_mesh {
    private:
        struct Settings {
            glm::uint verticesCount;
            glm::float32 step;
        };

        Settings settings_state;

        gl::GLuint settings{};
    public:
        viz_mesh_lines(const std::string &fieldCode);
        ~viz_mesh_lines();

        void drawFrame(std::size_t frame, const glm::mat4 &mvp);

        void recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) final;
    };

}// namespace viz

#endif //VIZ_INSTANCE_VIZ_MESH_LINES_H
