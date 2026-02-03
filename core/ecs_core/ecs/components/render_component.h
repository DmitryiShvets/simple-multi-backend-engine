#pragma once
#include <glm/glm.hpp>
#include <string>

namespace Core::Ecs::Component {

    struct Renderable {
        std::string mesh_id;
        std::string shader_id;
        glm::vec3 color = { 1.0f, 1.0f, 1.0f };
        bool visible = true;
    };
}