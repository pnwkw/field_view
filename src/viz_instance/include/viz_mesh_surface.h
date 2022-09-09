#ifndef VIZ_INSTANCE_VIZ_MESH_SURFACE_H
#define VIZ_INSTANCE_VIZ_MESH_SURFACE_H

#include <memory>
#include <utility>

#include <viz_mesh.h>

namespace viz {
    class viz_mesh_surface : public viz_mesh {
    private:
        gl::GLuint samples_buffer{};
        gl::GLuint keep_levels_buffer{};
        gl::GLuint mesh_level_buffer{};
        gl::GLuint mesh_index_buffer{};

        struct Settings {
            glm::float32 threshold;
        };

        Settings settings_state;

        gl::GLuint settings{};
    public:
        viz_mesh_surface(const std::string &fieldCode);
        ~viz_mesh_surface();

        void drawFrame(std::size_t frame, const glm::mat4 &mvp);

        void recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) final;
    };

}// namespace viz

#endif //VIZ_INSTANCE_VIZ_MESH_SURFACE_H
