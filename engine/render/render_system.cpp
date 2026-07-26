#include "render_system.h"
#include "core/gpu_types.h"
#include "core/uniforms.h"
#include "renderer.h"
#include "resource_manager.h"
#include "resources/descriptor_set.h"
#include "resources/uniform_block.h"
#include "scene_view.h"
#include "opengl_renderer.h"
#include "vulkan_renderer.h"
#include "d3d12_renderer.h"
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace ssme {
void RenderSystem::addBackend(GpuBackend type) {
  switch (type) {
  case GpuBackend::OpenGL:
    m_renderers.push_back(std::make_unique<OpenGLRenderer>(m_platform, m_rm));
    break;
  case GpuBackend::Vulkan:
    m_renderers.push_back(std::make_unique<VulkanRenderer>(m_platform, m_rm));
    break;
  case GpuBackend::DirectX12:
    m_renderers.push_back(std::make_unique<Dx12Renderer>(m_platform, m_rm));
    break;
  default:
    throw std::runtime_error("Unknown backend type");
  }
}

void RenderSystem::init(const std::vector<ImGuiContext *> &contexts) {
  if (contexts.size() != m_renderers.size()) {
    throw std::invalid_argument(
        "Number of contexts must match number of renderers");
  }
  for (size_t i = 0; i < m_renderers.size(); ++i) {
    if (m_renderers[i]) {
      m_renderers[i]->init(contexts[i]);
    }
  }
}

void RenderSystem::render(std::vector<SceneView> &scenes,
                          const std::vector<ImDrawData *> &ui_draw_data) {
  if (scenes.size() != m_renderers.size()) {
    throw std::invalid_argument(
        "Number of scenes must match number of renderers");
  }

  if (ui_draw_data.size() != m_renderers.size()) {
    throw std::invalid_argument(
        "Number of UI draw data must match number of renderers");
  }

  for (size_t i = 0; i < m_renderers.size(); ++i) {
    if (m_renderers[i]) {
        // if(i == 1) continue;
      m_renderers[i]->renderFrame(scenes[i], ui_draw_data[i]);
    }
  }
}

void RenderSystem::destroy() {
  for (auto &renderer : m_renderers) {
    if (renderer) {
      renderer->destroy();
    }
  }
  m_renderers.clear();
}

size_t RenderSystem::getRendererCount() const { return m_renderers.size(); }

IRenderer &RenderSystem::getRenderer(GpuBackend type) {
    for (auto &r : m_renderers)
        if (r && r->getGpuBackend() == type) return *r;
    throw std::out_of_range("Renderer not found");
}

const IRenderer& RenderSystem::getRenderer(GpuBackend type) const {
    for (auto& r : m_renderers)
        if (r && r->getGpuBackend() == type) return *r;
    throw std::out_of_range("Renderer not found");
}

RenderDevice &RenderSystem::getDevice(GpuBackend type) {
  return getRenderer(type).getRenderDeivce();
}


void RenderSystem::waitIdleAll() {
  for (auto &renderer : m_renderers) {
    renderer->waitIdle();
  }
}

void RenderSystem::createPerFrameResources() {
  // Skip if already created
  if (m_frame_data) {
    return;
  }
  m_frame_data = std::make_shared<FrameData>();

  // Create descriptor set layout for per-frame uniforms (Set 0)
  // Binding 0: GlobalUBO
  DescriptorLayout ds_layout_desc;
  ds_layout_desc.bindings.push_back({
      .binding = 0,
      .type = DescriptorType::UNIFORM_BUFFER,
      .count = 1,
      .stages = static_cast<uint32_t>(ShaderStage::VERTEX),
  });
  auto ds_layout_uuid = ds_layout_desc.uuid();
  m_frame_data->uniform_ds_layout =
      m_rm->load<DescriptorSetLayout>(ds_layout_uuid, ds_layout_desc);

  auto frame_count = MAX_FRAMES_IN_FLIGHT;
  m_frame_data->uniform_buffer.reserve(frame_count);
  m_frame_data->uniform_ds.reserve(frame_count);

  for (uint32_t i = 0; i < frame_count; i++) {
    auto u_desc = UniformBlockDesc{
        .name = "u_global_buffer" + std::to_string(i),
        .size = sizeof(Uniforms::FrameUniformsStd140),
        .data = {},
        .layout = {},
    };
    auto u_buffer = m_rm->load<UniformBuffer>(u_desc.name, u_desc);

    DescriptorDesc desc{
        .layout_id = m_frame_data->uniform_ds_layout.get()->getLayoutId(),
        .uniform_buffers = std::vector<RID>{u_buffer->getUbo()},
    };

    auto u_buffer_ds =
        m_rm->load<DescriptorSet>("u_global_ds" + std::to_string(i), desc);

    m_frame_data->uniform_buffer.push_back(std::move(u_buffer));
    m_frame_data->uniform_ds.push_back(std::move(u_buffer_ds));
  }
  for (size_t i = 0; i < m_renderers.size(); ++i) {
    if (m_renderers[i]) {
      m_renderers[i]->setFrameResources(m_frame_data);
    }
  }
}

} // namespace ssme
