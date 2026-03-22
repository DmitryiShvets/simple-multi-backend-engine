#pragma once

#include "core/resource_types.h"
#include "core/rid.h"
#include "resource.h"

namespace ssme {

class Mesh : public Resource {
public:
  static constexpr ResourceId ID = ResourceId::MESH;
  static constexpr uint32_t COMPONENTS = 2;

  Mesh(std::string id, const VecRID &rids, const VecRefRD &devices)
      : Resource(id, devices), m_vertex_buffer(rids[0]),
        m_index_buffer(rids[1]) {}

  // Этим методом пользуется Игрок/Загрузчик
  bool loadFromFile(const std::string &path) {
    // 1. Грузим данные с диска (через tinyobjloader или assimp)
    auto raw_data = GeometryLoader::load(path);

    // 2. Просим RenderDevice создать GPU-буферы
    // Мы получаем чистые RID, которые спрятаны внутри Mesh
    m_vertex_buffer = g_render_device->createBuffer(
        {.size = raw_data.vertices.size() * sizeof(Vertex),
         .usage = BufferUsage::VertexBuffer,
         .initial_data = raw_data.vertices.data()});

    m_index_buffer = g_render_device->createBuffer(
        {.size = raw_data.indices.size() * sizeof(uint32_t),
         .usage = BufferUsage::IndexBuffer,
         .initial_data = raw_data.indices.data()});

    m_index_count = raw_data.indices.size();
    return m_vertex_buffer.isValid() && m_index_buffer.isValid();
  }

  // Эти методы позовет Рендерер, когда придет время рисовать
  RID getVertexBuffer() const { return m_vertex_buffer; }
  RID getIndexBuffer() const { return m_index_buffer; }
  uint32_t getIndexCount() const { return m_index_count; }

private:
  RID m_vertex_buffer;
  RID m_index_buffer;
  uint32_t m_index_count = 0;
};

} // namespace ssme
