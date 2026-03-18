#pragma once

#include "vertex_layout.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// ============================================================================
// Vertex Types
// ============================================================================
namespace ssme {

/**
 * @brief Simple vertex with position only
 * Used by: default material
 */
struct VertexP {
  glm::vec3 position;

  /**
   * @brief Get vertex layout for this vertex type
   */
  static VertexLayout getLayout() {
    VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexP));
    layout.addPosition(0, 0, 0);
    return layout;
  }

  /**
   * @brief Get vertex size in bytes
   */
  static constexpr size_t size() { return sizeof(VertexP); }
};

/**
 * @brief Vertex with position and normal
 * Used by: ads material
 */
struct VertexPN {
  glm::vec3 position;
  glm::vec3 normal;

  /**
   * @brief Get vertex layout for this vertex type
   */
  static VertexLayout getLayout() {
    VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexPN));
    layout.addPosition(0, 0, 0);
    layout.addNormal(0, 1, offsetof(VertexPN, normal));
    return layout;
  }

  /**
   * @brief Get vertex size in bytes
   */
  static constexpr size_t size() { return sizeof(VertexPN); }
};

/**
 * @brief Vertex with position, normal and texture coordinates
 * Used by: textured materials
 */
struct VertexPNT {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coord;

  /**
   * @brief Get vertex layout for this vertex type
   */
  static VertexLayout getLayout() {
    VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexPNT));
    layout.addPosition(0, 0, 0);
    layout.addNormal(0, 1, offsetof(VertexPNT, normal));
    layout.addTexCoord(0, 2, offsetof(VertexPNT, tex_coord));
    return layout;
  }

  /**
   * @brief Get vertex size in bytes
   */
  static constexpr size_t size() { return sizeof(VertexPNT); }
};

/**
 * @brief Vertex with position, normal, texture coordinates and color
 * Used by: colored textured materials
 */
struct VertexPNTC {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coord;
  glm::vec4 color;

  /**
   * @brief Get vertex layout for this vertex type
   */
  static VertexLayout getLayout() {
    VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexPNTC));
    layout.addPosition(0, 0, 0);
    layout.addNormal(0, 1, offsetof(VertexPNTC, normal));
    layout.addTexCoord(0, 2, offsetof(VertexPNTC, tex_coord));
    layout.addColor(0, 3, offsetof(VertexPNTC, color));
    return layout;
  }

  /**
   * @brief Get vertex size in bytes
   */
  static constexpr size_t size() { return sizeof(VertexPNTC); }
};

} // namespace ssme
