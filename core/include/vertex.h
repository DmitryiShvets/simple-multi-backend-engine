#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "vertex_layout.h"


// ============================================================================
// Vertex Types
// ============================================================================

/**
 * @brief Simple vertex with position only
 * Used by: default material
 */
struct Vertex {
    glm::vec3 position;

    /**
     * @brief Get vertex layout for this vertex type
     */
    static Core::VertexLayout getLayout() {
        return Core::VertexLayout::createPositionOnly(sizeof(Vertex));
    }

    /**
     * @brief Get vertex size in bytes
     */
    static constexpr size_t size() { return sizeof(Vertex); }
};

/**
 * @brief Vertex with position and normal
 * Used by: ads material
 */
struct VertexN {
    glm::vec3 position;
    glm::vec3 normal;

    /**
     * @brief Get vertex layout for this vertex type
     */
    static Core::VertexLayout getLayout() {
        return Core::VertexLayout::createPositionNormal(sizeof(VertexN));
    }

    /**
     * @brief Get vertex size in bytes
     */
    static constexpr size_t size() { return sizeof(VertexN); }
};

/**
 * @brief Vertex with position, normal and texture coordinates
 * Used by: textured materials
 */
struct VertexNT {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;

    /**
     * @brief Get vertex layout for this vertex type
     */
    static Core::VertexLayout getLayout() {
        return Core::VertexLayout::createPositionNormalTex(sizeof(VertexNT));
    }

    /**
     * @brief Get vertex size in bytes
     */
    static constexpr size_t size() { return sizeof(VertexNT); }
};

/**
 * @brief Vertex with position, normal, texture coordinates and color
 * Used by: colored textured materials
 */
struct VertexNTC {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
    glm::vec4 color;

    /**
     * @brief Get vertex layout for this vertex type
     */
    static Core::VertexLayout getLayout() {
        return Core::VertexLayout::createPositionNormalTexColor(sizeof(VertexNTC));
    }

    /**
     * @brief Get vertex size in bytes
     */
    static constexpr size_t size() { return sizeof(VertexNTC); }
};
