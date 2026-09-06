#pragma once
#include "core/resource_types.h"
#include "core/vertex.h"
#include "utils/sphere.h"

#include <cstring>

namespace ssme {

class GeometryGenerator {
public:
  // Create simple triangle (VertexP - position only)
  static MeshDesc createTriangle() {
    MeshDesc desc;
    std::vector<VertexP> vertices = {
        {{0.0f, 0.5f, 0.0f}}, {{-0.5f, -0.5f, 0.0f}}, {{0.5f, -0.5f, 0.0f}}};

    // Pack to bytes
    desc.vertices.resize(vertices.size() * sizeof(VertexP));
    std::memcpy(desc.vertices.data(), vertices.data(), desc.vertices.size());

    desc.indices = {0, 1, 2};
    desc.layout = VertexP::getLayout();
    desc.bounds = {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}};
    return desc;
  }

  // Create sphere (VertexPNT - position + normal + uv)
  static MeshDesc createSphere(float radius, int sectors, int stacks) {
    MeshDesc desc;
    // Use existing SphereGeometry
    SphereGeometry sphere = getSphere3D(radius, sectors, stacks);

    std::vector<VertexPNT> vertices;
    vertices.reserve(sphere.positions.size());

    for (size_t i = 0; i < sphere.positions.size(); ++i) {
      vertices.push_back(
          {sphere.positions[i], sphere.normals[i], sphere.text_coords[i]});
    }

    desc.vertices.resize(vertices.size() * sizeof(VertexPNT));
    std::memcpy(desc.vertices.data(), vertices.data(), desc.vertices.size());

    desc.indices = sphere.indices; // Now just copy indices!
    desc.layout = VertexPNT::getLayout();

    // Calculate bounds
    desc.bounds = {{-radius, -radius, -radius}, {radius, radius, radius}};
    return desc;
  }

  // Create quad (VertexPNT - position + normal + texcoord)
  static MeshDesc createQuad() {
    MeshDesc desc;
    std::vector<VertexPNT> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    };
    desc.vertices.resize(vertices.size() * sizeof(VertexPNT));
    std::memcpy(desc.vertices.data(), vertices.data(), desc.vertices.size());
    desc.indices = {0, 1, 2, 0, 2, 3};
    desc.layout = VertexPNT::getLayout();
    desc.bounds = {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}};
    return desc;
  }
};

// Stub for file loader
class GeometryLoader {
public:
  static MeshDesc loadFromFile(const std::string &path) {
    // For now return empty triangle to avoid crashes
    // In the future tinyobjloader will be here
    return GeometryGenerator::createTriangle();
  }
};

} // namespace ssme
