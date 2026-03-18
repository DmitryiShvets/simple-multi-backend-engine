#pragma once
#include "vertex.h"
#include "vertex_layout.h"
#include <vector>

namespace Core::Ecs::Component {

    /**
     * @brief Geometry with position-only vertices (Vertex)
     * Used by: default material
     */
    struct Geometry {
        std::vector<Vertex> vertices;
        static Core::VertexLayout getLayout() {
            return Vertex::getLayout();
        }
    };

    /**
     * @brief Geometry with position+normal vertices (VertexN)
     * Used by: ads material
     */
    struct GeometryN {
        std::vector<VertexN> vertices;
        static Core::VertexLayout getLayout() {
            return VertexN::getLayout();
        }
    };

    /**
     * @brief Geometry with position+normal+texcoord vertices (VertexNT)
     * Used by: textured materials
     */
    struct GeometryNT {
        std::vector<VertexNT> vertices;
        static Core::VertexLayout getLayout() {
            return VertexNT::getLayout();
        }
    };

    /**
     * @brief Geometry with position+normal+texcoord+color vertices (VertexNTC)
     * Used by: colored textured materials
     */
    struct GeometryNTC {
        std::vector<VertexNTC> vertices;
        static Core::VertexLayout getLayout() {
            return VertexNTC::getLayout();
        }
    };
}
