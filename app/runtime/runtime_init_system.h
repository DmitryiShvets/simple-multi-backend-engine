#pragma once
#include "concepts.h"
#include "ecs/components/geometry_component.h"
#include "ecs/components/material_component.h"
#include "ecs/components/runtime_component.h"
#include "render_device.h"
#include "render_types.h"
#include "vertex.h"

#include <vector>

namespace Core::Ecs::System {

template <typename WorldType>
  requires EcsWorld<WorldType>
class RuntimeInitSystem {
public:
  RuntimeInitSystem(WorldType &world, Render::Device &vk_device,
                    Render::Device &gl_device)
      : m_world(world), m_vk_device(vk_device), m_gl_device(gl_device),
        m_query(world.template createQuery<const Component::Geometry,
                                           const Component::Material>()) {}

  void initialize() {
    // --- Create a shared Vulkan Layout ---
    Render::DescriptorSetLayoutDesc ds_layout_desc;
    ds_layout_desc.bindings = {
        {0, Render::DescriptorType::UNIFORM_BUFFER,
         static_cast<uint32_t>(Render::ShaderStage::VERTEX)},
        {1, Render::DescriptorType::COMBINED_IMAGE_SAMPLER,
         static_cast<uint32_t>(Render::ShaderStage::FRAGMENT)}};
    RID vk_ds_layout_rid = m_vk_device.createDescriptorSetLayout(ds_layout_desc);

    Render::PipelineLayoutDesc pl_layout_desc = {
        .descriptor_set_layouts = {vk_ds_layout_rid}};
    RID vk_pl_layout_rid = m_vk_device.createPipelineLayout(pl_layout_desc);

    // --- Create resources for each renderable entity ---
    struct Command {
        EntityHandle entity;
        RID vk_geom;
        RID gl_geom;
        RID vk_mat;
        RID gl_mat;
    };
    std::vector<Command> commands;

    m_query.each([this, vk_pl_layout_rid, &commands](EntityHandle entity,
                                const Component::Geometry &geom,
                                const Component::Material &mat) {

      // 1. Create Vertex Buffer for both APIs
      Render::BufferDesc buffer_desc{
          .size = sizeof(geom.vertices[0]) * geom.vertices.size(),
          .usage = static_cast<uint32_t>(Render::BufferUsage::VERTEX_BUFFER),
          .initial_data = (void *)geom.vertices.data()};
      RID vk_geom_buff = m_vk_device.createBuffer(buffer_desc);
      RID gl_geom_buff = m_gl_device.createBuffer(buffer_desc);

      // 2. Create Vulkan Pipeline
      Render::GraphicsPipelineDesc vk_pipeline_desc{
          .name = mat.mat_name,
          .pipeline_layout_rid = vk_pl_layout_rid,
          .shader_modules = {{"res/shaders/v_test.vert.spv",
                              Render::ShaderStage::VERTEX},
                             {"res/shaders/f_test.frag.spv",
                              Render::ShaderStage::FRAGMENT}},
          .vertex_input_state = {
              .bindings = {{.binding = 0, .stride = sizeof(Vertex)}},
              .attributes = {{.location = 0,
                              .binding = 0,
                              .format = Render::Format::R32G32B32_SFLOAT,
                              .offset = offsetof(Vertex, position)},
                             {.location = 1,
                              .binding = 0,
                              .format = Render::Format::R32G32B32_SFLOAT,
                              .offset = offsetof(Vertex, color)}},
          }};
      RID vk_mat_id = m_vk_device.createGraphicsPipeline(vk_pipeline_desc);

      // 3. Create OpenGL Pipeline (Shader Program)
      Render::GraphicsPipelineDesc gl_pipeline_desc{
          .name = mat.mat_name,
          .shader_modules = {{"res/shaders/f_default.glsl",
                              Render::ShaderStage::FRAGMENT},
                             {"res/shaders/v_default.glsl",
                              Render::ShaderStage::VERTEX}},
      };
      RID gl_mat_id = m_gl_device.createGraphicsPipeline(gl_pipeline_desc);

      // 4. Defer component addition
      commands.push_back({entity, vk_geom_buff, gl_geom_buff, vk_mat_id, gl_mat_id});
    });

    // 5. Apply component changes after query is finished
    for (const auto& cmd : commands) {
        m_world.template addComponent<Component::VkRuntime>(cmd.entity, cmd.vk_geom, cmd.vk_mat);
        m_world.template addComponent<Component::GlRuntime>(cmd.entity, cmd.gl_geom, cmd.gl_mat);
    }
  }

private:
  WorldType &m_world;
  Render::Device &m_vk_device;
  Render::Device &m_gl_device;
  decltype(m_world.template createQuery<const Component::Geometry,
                                        const Component::Material>()) m_query;
};

} // namespace Core::Ecs::System
