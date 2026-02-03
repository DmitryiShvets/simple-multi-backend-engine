#pragma once
#include "opengl_buffer_objects.h"

#include <string>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>

#include "i_renderer.h"
#include "opengl_resource_manager.h"

//class OpenglResourceManager;

class OpenGLRenderer : public Render::IRenderer {
public:
    void initialize(int width, int height) override;
    void destroy() override;
    // TODO: DELETE IT
    void drawBundle(const std::string& shader, void* bundle) override;
    void frame(float delta_time) override;
    void resize(int width, int height) override;

    void clear() override;
    void setViewPort(int x, int y, int width, int height) override;
    void setClearColor(float r, float g, float b, float a) override;
private:
    static void draw(const VAO& vao, const EBO& ebo);
    static void draw(const VAO& vao);
    static void draw(VAO* vao);
    static void draw(VAO* vao, EBO* ebo);
    std::unique_ptr<OpenglResourceManager> m_resource_manager;
};

struct RenderObject
{
    RenderObject(const std::string& uuid, const std::string& shaderId, const glm::vec3& color, const std::vector<glm::vec3>& coordsArray);
    std::string uuid;
    std::string shaderId;
    glm::vec3 color;

    VAO vao;
};
