#pragma once
#include "vertex.h"
#include <vector>

namespace Core::Ecs::Component {

    struct Geometry {
        std::vector<VertexN> vertices;
    };
}
