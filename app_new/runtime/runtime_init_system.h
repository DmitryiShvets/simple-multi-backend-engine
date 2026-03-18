#pragma once
#include "concepts.h"
#include "ecs/components/geometry_component.h"
#include "ecs/components/material_component.h"
#include "ecs/components/runtime_component.h"
#include "ecs/components/transform_component.h"
#include "logger.h"
#include "render_device.h"
#include "render_manager.h"
#include "uniform_factory_registry.h"
#include "uniforms.h"
#include "vertex_layout.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Core::Ecs::System {

// ============================================================================
// Helper Functions
// ============================================================================

inline RID createVertexBuffer(Render::RenderDevice &device, const void *data,
                              size_t size, const VertexLayout &vertex_layout) {
  Render::BufferDesc buffer_desc{
      .size = size,
      .usage = static_cast<uint32_t>(Render::BufferUsage::VERTEX_BUFFER),
      .is_host_visible = true,
      .initial_data = (void *)data,
      .vertex_layout = vertex_layout};

  return device.createBuffer(buffer_desc);
}

// ============================================================================
// RuntimeInitSystem - creates GPU resources for geometry and materials
// ============================================================================

/**
 * @brief System that creates GPU resources for ECS entities
 *
 * Architecture:
 * 1. First pass: create vertex buffers for all geometry
 * 2. Second pass: create material resources using factories from registry
 *
 * The system does NOT know about specific material types.
 * Material-specific logic is provided by Render::UniformFactoryRegistry.
 */
template <typename WorldType>
  requires EcsWorld<WorldType>
class RuntimeInitSystem {
public:
  RuntimeInitSystem(WorldType &world, Render::RenderManager &render)
      : m_world(world), 
        m_gl_device(render.getDevice(Core::BackendType::OpenGL)),
        m_vk_device(render.getDevice(Core::BackendType::Vulkan)) {}

  void initialize() {
    // First pass: create vertex buffers for all geometry
    auto geometry_commands = process_vertex_buffers();

    // Second pass: create material resources using registry
    // No hardcoded material types!
    process_materials(geometry_commands);
  }

private:
  struct GeometryCommand {
    EntityHandle entity;
    RID vk_geom;
    RID gl_geom;
  };

  // Process all entities with Geometry component and create vertex buffers
  std::vector<GeometryCommand> process_vertex_buffers() {
    std::vector<GeometryCommand> commands;

    // Process each geometry type
    process_geometry_type<Component::Geometry>(commands);
    process_geometry_type<Component::GeometryN>(commands);
    process_geometry_type<Component::GeometryNT>(commands);
    process_geometry_type<Component::GeometryNTC>(commands);

    return commands;
  }

  // Template helper to process a geometry type (avoids code duplication)
  template <typename GeometryComponent>
  void process_geometry_type(std::vector<GeometryCommand> &commands) {
    auto query = m_world.template createQuery<const GeometryComponent>();

    query.each([this, &commands](EntityHandle entity,
                                 const GeometryComponent &geom) {
      const auto vertex_layout = GeometryComponent::getLayout();
      uint64_t total_size = vertex_layout.getStride() * geom.vertices.size();

      RID vk_geom = createVertexBuffer(m_vk_device, geom.vertices.data(),
                                       total_size, vertex_layout);
      RID gl_geom = createVertexBuffer(m_gl_device, geom.vertices.data(),
                                       total_size, vertex_layout);

      commands.push_back({entity, vk_geom, gl_geom});
    });
  }

  // Process all materials using factories from registry
  void
  process_materials(const std::vector<GeometryCommand> &geometry_commands) {
    // Create geometry lookup map
    std::unordered_map<EntityHandle, GeometryCommand> geom_map;
    for (const auto &gc : geometry_commands) {
      geom_map[gc.entity] = gc;
    }

    // Process each material type
    process_material_type<Component::DefaultMaterial>(geometry_commands,
                                                      geom_map);
    process_material_type<Component::AdsMaterial>(geometry_commands, geom_map);
    // Add new material types here:
    // process_material_type<Component::PbrMaterial>(geometry_commands,
    // geom_map);
  }

  // Process a single material type using factory from registry
  template <typename MaterialComponent>
  void process_material_type(
      const std::vector<GeometryCommand> &geometry_commands,
      const std::unordered_map<EntityHandle, GeometryCommand> &geom_map) {

    const std::string &material_type = MaterialComponent::material_type_name;

    // Get factory for this material type
    const auto *factory =
        Render::UniformFactoryRegistry::instance().getFactory(material_type);
    if (!factory) {
      Logger::error_log("Unknown material type.  No factory registered");
      return;
    }

    // Get pipeline config for this material to get vertex layout
    const auto *vk_config = m_vk_device.getPipelineConfig(material_type);
    const auto *gl_config = m_gl_device.getPipelineConfig(material_type);

    if (!vk_config || !gl_config) {
      // No pipeline config for this material - skip
      return;
    }

    struct Command {
      EntityHandle entity;
      RID vk_material_template;
      RID gl_material_template;
      RID vk_per_object_data;
      RID gl_per_object_data;
      RID vk_geom;
      RID gl_geom;
    };
    std::vector<Command> commands;

    // Query all entities with this material type (regardless of geometry type)
    auto query = m_world.template createQuery<const MaterialComponent,
                                              const Component::Transform>();

    query.each([&, this](EntityHandle entity, const MaterialComponent &mat,
                         const Component::Transform &transform) {
      // Get geometry IDs from lookup map
      auto geom_it = geom_map.find(entity);
      if (geom_it == geom_map.end())
        return;

      RID vk_geom = geom_it->second.vk_geom;
      RID gl_geom = geom_it->second.gl_geom;

      // Create material uniforms using factory
      UniformSet mat_uniforms = (*factory)(&mat);

      // Create material template (cached, contains pipeline)
      RID vk_mat_tpl = m_vk_device.createMaterial(material_type, mat_uniforms);
      RID gl_mat_tpl = m_gl_device.createMaterial(material_type, mat_uniforms);

      // Create per-object uniform buffer ONLY if material has
      // object_uniform_layout
      RID vk_per_obj = RID::INVALID;
      RID gl_per_obj = RID::INVALID;

      if (vk_config->object_uniform_layout.getTotalSize() > 0) {
        glm::mat4 model_mat = transform.getModelMatrix();
        glm::mat3 normal_mat =
            glm::transpose(glm::inverse(glm::mat3(model_mat)));

        Uniforms::ObjectUniforms obj_uniforms{.model_matrix = model_mat,
                                              .normal_matrix = normal_mat};
        auto packed_uniforms =
            Uniforms::ObjectUniformsStd140::from(obj_uniforms);

        vk_per_obj = m_vk_device.createBuffer(Render::BufferDesc{
            .size = vk_config->object_uniform_layout.getTotalSize(),
            .usage = static_cast<uint32_t>(Render::BufferUsage::UNIFORM_BUFFER),
            .is_host_visible = true,
            .initial_data = &packed_uniforms});
        gl_per_obj = m_gl_device.createBuffer(Render::BufferDesc{
            .size = gl_config->object_uniform_layout.getTotalSize(),
            .usage = static_cast<uint32_t>(Render::BufferUsage::UNIFORM_BUFFER),
            .is_host_visible = true,
            .initial_data = &packed_uniforms});
      }

      commands.push_back({entity, vk_mat_tpl, gl_mat_tpl, vk_per_obj,
                          gl_per_obj, vk_geom, gl_geom});
    });

    // Create VkRuntime and GlRuntime components
    for (const auto &cmd : commands) {
      auto *vk_mat = m_vk_device.getMaterial(cmd.vk_material_template);
      auto *gl_mat = m_gl_device.getMaterial(cmd.gl_material_template);

      // Create descriptor set for object uniforms ONLY if layout exists
      RID vk_obj_ds = RID::INVALID;
      RID gl_obj_ds = RID::INVALID;

      if (cmd.vk_per_object_data.isValid() &&
          vk_mat->render_data.object_uniform_ds_layout.isValid()) {
        vk_obj_ds = m_vk_device.createDescriptorSet(
            vk_mat->render_data.object_uniform_ds_layout,
            {cmd.vk_per_object_data});
      }
      if (cmd.gl_per_object_data.isValid() &&
          gl_mat->render_data.object_uniform_ds_layout.isValid()) {
        gl_obj_ds = m_gl_device.createDescriptorSet(
            gl_mat->render_data.object_uniform_ds_layout,
            {cmd.gl_per_object_data});
      }

      m_world.template addComponent<Component::VkRuntime>(
          cmd.entity, Component::VkRuntime{
                          .geometry_id = cmd.vk_geom,
                          .material_id = cmd.vk_material_template,
                          .obj_uniform_id = cmd.vk_per_object_data,
                          .obj_uniform_ds = vk_obj_ds,
                          .material_type = material_type,
                          .visible = true,
                      });

      m_world.template addComponent<Component::GlRuntime>(
          cmd.entity,
          Component::GlRuntime{.geometry_id = cmd.gl_geom,
                               .material_id = cmd.gl_material_template,
                               .obj_uniform_id = cmd.gl_per_object_data,
                               .obj_uniform_ds = gl_obj_ds,
                               .material_type = material_type,
                               .visible = true});
    }
  }

  WorldType &m_world;
  Render::RenderDevice &m_vk_device;
  Render::RenderDevice &m_gl_device;
};

} // namespace Core::Ecs::System
