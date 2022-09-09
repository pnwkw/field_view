#ifndef VIZ_INSTANCE_VIZ_MESH_RIBBONS_H
#define VIZ_INSTANCE_VIZ_MESH_RIBBONS_H

#include <memory>
#include <utility>

#include <viz_mesh.h>

namespace viz {
    class viz_mesh_ribbons : public viz_mesh {
    private:
        struct Settings {
            glm::uint segmentsCount;
            glm::float32 initialOffset;
            glm::float32 stepSize;
        };

        Settings settings_state;

        gl::GLuint settings{};
    public:
        viz_mesh_ribbons(const std::string &fieldCode);
        ~viz_mesh_ribbons();

        void drawFrame(std::size_t frame, const glm::mat4 &mvp);

        void recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) final;
    };

}// namespace viz

#endif //VIZ_INSTANCE_VIZ_MESH_RIBBONS_H
