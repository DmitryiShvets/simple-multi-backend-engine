#pragma once

#include "vertex_layout.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
    Core::VertexLayout layout;
    layout.setSingleBinding(sizeof(Vertex));
    layout.addPosition(0, 0, 0);
    return layout;
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
    Core::VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexN));
    layout.addPosition(0, 0, 0);
    layout.addNormal(0, 1, offsetof(VertexN, normal));
    return layout;
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
    Core::VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexNT));
    layout.addPosition(0, 0, 0);
    layout.addNormal(0, 1, offsetof(VertexNT, normal));
    layout.addTexCoord(0, 2, offsetof(VertexNT, tex_coord));
    return layout;
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
    Core::VertexLayout layout;
    layout.setSingleBinding(sizeof(VertexNTC));
    layout.addPosition(0, 0, 0);
    layout.addNormal(0, 1, offsetof(VertexNTC, normal));
    layout.addTexCoord(0, 2, offsetof(VertexNTC, tex_coord));
    layout.addColor(0, 3, offsetof(VertexNTC, color));
    return layout;
  }

  /**
   * @brief Get vertex size in bytes
   */
  static constexpr size_t size() { return sizeof(VertexNTC); }
};
