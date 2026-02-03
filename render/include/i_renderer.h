#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Render {

    struct Renderable {
        std::string mesh_id;
        std::string shader_id;
        glm::vec3 color = {1.0f, 1.0f, 1.0f};
        bool visible = true;
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual void initialize(int width, int height) = 0;
        virtual void destroy() = 0;
        virtual void drawBundle(const std::string& shader, void* bundle) = 0;
        virtual void frame(float delta_time) = 0;
        virtual void resize(int width, int height) = 0;

        virtual void clear() = 0;
        virtual void setViewPort(int x, int y, int width, int height) = 0;
        virtual void setClearColor(float r, float g, float b, float a) = 0;
    };

} // namespace Render 