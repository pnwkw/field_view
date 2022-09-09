#ifndef VIZ_INSTANCE_VIZ_MESH_H
#define VIZ_INSTANCE_VIZ_MESH_H

#include <memory>
#include <utility>

#include <glsl.h>

namespace viz {
    class viz_mesh {
    protected:
        gl::GLuint vertex_buffer{};

        std::unique_ptr<glsl> handler{};

        gl::GLuint ubo{};

        gl::GLuint task_shader;
        gl::GLuint mesh_shader;
        gl::GLuint frag_shader;

        gl::GLuint program;

        using State = struct {
            glm::mat4 MVP;
        };

        State state{};

        struct Point {
            glm::vec4 pos;
        };

        std::vector<decltype(Point::pos)> points;
    public:
        viz_mesh();
        virtual ~viz_mesh();

        void generatePoints();

        virtual void drawFrame(std::size_t frame, const glm::mat4 &mvp) = 0;

        virtual void recompileShaders(const std::vector<std::reference_wrapper<std::string>> &codes) = 0;
    };

}// namespace viz

#endif //VIZ_INSTANCE_VIZ_MESH_H
