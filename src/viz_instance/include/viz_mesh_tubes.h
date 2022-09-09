#ifndef VIZ_INSTANCE_VIZ_MESH_TUBES_H
#define VIZ_INSTANCE_VIZ_MESH_TUBES_H

#include <memory>
#include <utility>

#include <viz_mesh.h>

namespace viz {
    class viz_mesh_tubes : public viz_mesh {
    private:
        struct Settings {
            glm::uint segmentsCount;
            glm::uint circleVertices;
            glm::float32 radius;
            glm::float32 stepSize;
        };

        Settings settings_state;

        gl::GLuint settings{};
    public:
        viz_mesh_tubes(const std::string &fieldCode);
        ~viz_mesh_tubes();

        void drawFrame(std::size_t frame, const glm::mat4 &mvp);

        void recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) final;
    };

}// namespace viz

#endif //VIZ_INSTANCE_VIZ_MESH_TUBES_H
