#include "render_proxy_builder.h"
#include "core/uniform_set.h"
#include "ecs/components/material_component.h"
#include "ecs/components/mesh_component.h"
#include "ecs/components/transform_component.h"
#include "render_proxy.h"
#include "resource_manager.h"

namespace ssme {

RenderProxyBuilder::RenderProxyBuilder(ResourceManager &rm)
    :  m_rm(rm) {}

void RenderProxyBuilder::buildProxy(Entity &entity) {
  const auto &mesh_handle = entity.get<MeshComponent>()->handle;
  const auto &mat_handle = entity.get<MaterialComponent>()->handle;
  // const auto &render_proxy = entity.get<RenderProxy>();
  auto ent_mame = entity.name();
  auto *mesh_res = mesh_handle.get();
  auto *mat_res = mat_handle.get();
  // auto &proxy = *render_proxy;
  RenderProxy proxy;
  proxy.render_item.ent = entity.id();
  proxy.render_item.vertex_buffers.push_back(mesh_res->getVertexBuffer());
  proxy.render_item.index_buffer = mesh_res->getIndexBuffer();
  proxy.render_item.draw_cmd.index_count = mesh_res->getIndexCount();
  const auto &vert_shader = mat_res->getVertShader();
  const auto &frag_Shader = mat_res->getFragShader();
  const auto &vertex_layout = mesh_res->getVertexLayout();
  // Material data (Pipeline + Descriptor Set 1)
  PipelineParams params{
      .vert_shader = vert_shader,
      .frag_Shader = frag_Shader,
      .vertex_layout = mesh_res->getVertexLayout(),
      // TODO: (MAKE IT DATA DRIVEN) set these parameters from entity data
      // or render proxy
      .rasterization_state = RasterizationStateDesc{},
      .depth_stencil_state = DepthStencilStateDesc{
          .depthTestEnable = true,
          .depthWriteEnable = true
      },
  };
  auto pipeline_uuid = params.uuid();
  proxy.pipeline_ref = m_rm.load<Pipeline>(pipeline_uuid, params);
  proxy.render_item.pipeline = proxy.pipeline_ref.get()->getPipelineID();

  // TODO: compare it with vertex_layout
  auto req = vert_shader->getVertexInputRequirements();
  UniformSet object_ubo_data;
  auto object_ubo_layout = vert_shader->getLayout("ObjectUBO");
  m_packer.fillUniformSet(entity, object_ubo_data, *object_ubo_layout);

  // Push constants (data-driven from shader reflection)
  for (const auto &[push_name, push_range] :
       vert_shader->getPushConstantsMap()) {
    auto push_layout = vert_shader->getLayout(push_name);
    if (!push_layout) {
      continue;
    }
    UniformSet push_set = vert_shader->createUniformSet(push_name);
    m_packer.fillUniformSet(entity, push_set, *push_layout);
    for (const auto &var : push_layout->getVariables()) {
      UniformValue value = UniformValue::createDefault(var.type);
      if (const UniformValue *src = push_set.get(var.name)) {
        value = *src;
      }
      value.setLabel(var.name);
      proxy.render_item.push_constants.emplace(var.name, value);
      proxy.render_item.push_constants_offsets.emplace(
          var.name,
          static_cast<uint32_t>(push_range.offset + var.offset));
    }
  }

  // creation gpu unifoms for object phase
  DescriptorLayout ds_layout_desc;
  ds_layout_desc.bindings.push_back({
      .binding = 0,
      .type = DescriptorType::UNIFORM_BUFFER,
      .count = 1,
      .stages = static_cast<uint32_t>(ShaderStage::VERTEX),
  });
  proxy.obj_uniform_ds_layout =
      m_rm.load<DescriptorSetLayout>(ds_layout_desc.uuid(), ds_layout_desc);

  auto u_buff_desc = UniformBlockDesc{
      .name = "ObjectUBO",
      .size = object_ubo_layout->getTotalSize(),
      .data = object_ubo_layout->pack(object_ubo_data),
      .layout = object_ubo_layout,
  };
  proxy.obj_uniforms = m_rm.load<UniformBuffer>("u_obj_buffer" + ent_mame, u_buff_desc);

  DescriptorDesc u_ds_desc{
      .layout_id = proxy.obj_uniform_ds_layout.get()->getLayoutId(),
      .uniform_buffers = std::vector<RID>{proxy.obj_uniforms->getUbo()},
  };
  proxy.obj_uniform_ds = m_rm.load<DescriptorSet>("u_obj_ds" + ent_mame, u_ds_desc);

  // binding phase
  proxy.render_item.setDescriptor(1,
                                  proxy.obj_uniform_ds->getDescriptorSetId());
  const auto &mat_descriptors = mat_res->getDescriptors();
  int j = 2;
  for (int i = 0; i < mat_descriptors.size(); i++, j++) {
    const auto &id = mat_descriptors[i];
    proxy.render_item.setDescriptor(j, id);

  }
  if (auto tex_ds = mat_res->getTextureDescriptor(); tex_ds.isValid()) {
    proxy.render_item.setDescriptor(j, tex_ds);
  }
  proxy.local_aabb = mesh_res->getBounds();
  entity.add(std::move(proxy));
}

void RenderProxyBuilder::ensureConsistency(const Entity &entity) {}

} // namespace ssme
