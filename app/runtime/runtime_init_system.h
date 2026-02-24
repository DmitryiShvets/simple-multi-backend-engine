#pragma once
#include "concepts.h"
#include "ecs/components/geometry_component.h"
#include "ecs/components/material_component.h"
#include "ecs/components/runtime_component.h"
#include "ecs/components/transform_component.h"
#include "render_device.h"
#include "uniforms.h"

#include <unordered_map>
#include <vector>

namespace Core::Ecs::System {

inline RID createVertexBuffer(Render::Device &device, const void *data, size_t size) {
  Render::BufferDesc buffer_desc{
      .size = size,
      .usage = static_cast<uint32_t>(Render::BufferUsage::VERTEX_BUFFER),
      .is_host_visible = true,
      .initial_data = (void *)data};


  return device.createBuffer(buffer_desc);
}

// RuntimeInitSystem - creates GPU resources for geometry and materials
template <typename WorldType>
  requires EcsWorld<WorldType>
class RuntimeInitSystem {
public:
  RuntimeInitSystem(WorldType &world, Render::Device &gl_device,
                    Render::Device &vk_device)
      : m_world(world), m_gl_device(gl_device), m_vk_device(vk_device) {}

  void initialize() {
    // First pass: create vertex buffers for all geometry
    auto geometry_commands = process_vertex_buffers();

    // Second pass: create material templates and uniform buffers for each
    // material type
    process_material_type<Component::DefaultMaterial>(geometry_commands);
    // Add new material types here:
    // process_material_type<Component::PbrMaterial>();
  }

private:
  struct GeometryCommand {
    EntityHandle entity;
    RID vk_geom;
    RID gl_geom;
  };

  // Process all entities with Geometry component and create vertex buffers
  std::vector<GeometryCommand> process_vertex_buffers() {
    auto query = m_world.template createQuery<const Component::Geometry>();

    std::vector<GeometryCommand> commands;

    query.each([this, &commands](EntityHandle entity,
                                 const Component::Geometry &geom) {
      RID vk_geom =
          createVertexBuffer(m_vk_device, geom.vertices.data(),
                             sizeof(geom.vertices[0]) * geom.vertices.size());
      RID gl_geom =
          createVertexBuffer(m_gl_device, geom.vertices.data(),
                             sizeof(geom.vertices[0]) * geom.vertices.size());

      commands.push_back({entity, vk_geom, gl_geom});
    });

    return commands;
  }

  // Process all entities with specific material type
  // Creates material template (once per type, cached) and uniform buffer (per
  // object)
  template <typename MaterialComponent>
  void
  process_material_type(const std::vector<GeometryCommand> &geometry_commands) {
    auto query = m_world.template createQuery<const Component::Geometry,
                                              const MaterialComponent,
                                              const Component::Transform>();

    struct Command {
      EntityHandle entity;
      RID vk_material_template;
      RID gl_material_template;
      Core::Uniforms::ObjectUniforms vk_per_object_data;     // Per-object uniforms (model matrix)
      Core::Uniforms::ObjectUniforms gl_per_object_data;
      RID vk_geom;
      RID gl_geom;
    };
    std::vector<Command> commands;

    // Create a map for quick geometry lookup
    std::unordered_map<EntityHandle, GeometryCommand> geom_map;
    for (const auto &gc : geometry_commands) {
      geom_map[gc.entity] = gc;
    }

    query.each([this, &commands, &geom_map](
                   EntityHandle entity, const Component::Geometry & /*geom*/,
                   const MaterialComponent &mat,
                   const Component::Transform &transform) {
      // Get geometry IDs from first pass
      auto geom_it = geom_map.find(entity);
      RID vk_geom = geom_it != geom_map.end() ? geom_it->second.vk_geom : RID{};
      RID gl_geom = geom_it != geom_map.end() ? geom_it->second.gl_geom : RID{};

      // Create/get material template (cached, contains pipeline)
      // Create material with uniform data
      Core::UniformSet mat_uniforms;
      mat_uniforms.set("color", Core::UniformValue(mat.color));

      RID vk_mat_tpl =
          m_vk_device.createMaterial(MaterialComponent::material_type_name, mat_uniforms);
      RID gl_mat_tpl =
          m_gl_device.createMaterial(MaterialComponent::material_type_name, mat_uniforms);

      // Get the material to access uniform buffer RID
      // Per-object uniform buffer (unique per object, contains model matrix)
      Core::Uniforms::ObjectUniforms obj_uniforms{
          .model_matrix = transform.getModelMatrix()};
      // RID vk_per_obj = m_vk_device.createBuffer(Render::BufferDesc{
      //     .size = sizeof(Core::Uniforms::ObjectUniforms),
      //     .usage = static_cast<uint32_t>(Render::BufferUsage::UNIFORM_BUFFER),
      //     .is_host_visible = true,
      //     .initial_data = &obj_uniforms
      // });
      // RID gl_per_obj = m_gl_device.createBuffer(Render::BufferDesc{
      //     .size = sizeof(Core::Uniforms::ObjectUniforms),
      //     .usage = static_cast<uint32_t>(Render::BufferUsage::UNIFORM_BUFFER),
      //     .is_host_visible = true,
      //     .initial_data = &obj_uniforms
      // });

      commands.push_back(
          {entity, vk_mat_tpl, gl_mat_tpl,
           obj_uniforms, obj_uniforms,
           vk_geom, gl_geom});
    });

    // Create VkRuntime and GlRuntime components with all data
    for (const auto &cmd : commands) {
      m_world.template addComponent<Component::VkRuntime>(
          cmd.entity,
          Component::VkRuntime{.geometry_id = cmd.vk_geom,
                               .material_id = cmd.vk_material_template,
                               .data_id = cmd.vk_per_object_data,
                               .material_type =
                                   MaterialComponent::material_type_name,
                               .visible = true});

      m_world.template addComponent<Component::GlRuntime>(
          cmd.entity,
          Component::GlRuntime{.geometry_id = cmd.gl_geom,
                               .material_id = cmd.gl_material_template,
                               .data_id = cmd.gl_per_object_data,
                               .material_type =
                                   MaterialComponent::material_type_name,
                               .visible = true});
    }
  }

  WorldType &m_world;
  Render::Device &m_vk_device;
  Render::Device &m_gl_device;
};

} // namespace Core::Ecs::System
