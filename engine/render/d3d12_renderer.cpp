#include "d3d12_renderer.h"
#include "com_exception.h"
#include "core/render_types.h"
#include "core/resource_types.h"
#include "core/uniforms.h"
#include "d3dx12.h"
#include "dx12_command_list.h"
#include "dx12_device.h"
#include "dx12_render_device.h"
#include "dx12_swap_chain.h"
#include "graph/render_graph.h"
#include "render_device.h"
#include "render_item.h"
#include "resource_manager.h"
#include "scene_view.h"
#include "utils/logger.h"
#include <backends/imgui_impl_dx12.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <utils/curve_utils.h>

namespace {
void Dx12ImguiSrvAlloc(ImGui_ImplDX12_InitInfo *info,
                       D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu,
                       D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu) {
  auto *h = static_cast<ssme::Dx12Renderer::ImGuiSrvHandle *>(info->UserData);
  *out_cpu = h->cpu;
  *out_gpu = h->gpu;
  h->allocated = true;
}
void Dx12ImguiSrvFree(ImGui_ImplDX12_InitInfo *info,
                      D3D12_CPU_DESCRIPTOR_HANDLE,
                      D3D12_GPU_DESCRIPTOR_HANDLE) {
  auto *h = static_cast<ssme::Dx12Renderer::ImGuiSrvHandle *>(info->UserData);
  h->allocated = false;
}
} // namespace
namespace ssme {


Dx12Renderer::Dx12Renderer(Platform *platform, ResourceManager *rm)
    : m_platform(platform), m_rm(rm) {
  /* -------------INIT STATE-------------- */
  m_device = std::make_unique<ssme::d3d12::Dx12Device>(platform);
  m_rhi_device =
      std::make_unique<ssme::d3d12::Dx12RenderDevice>(*m_device, m_storage);
  m_swap_chain = std::make_unique<ssme::d3d12::Dx12SwapChain>(
      *m_device, Extent2D{800, 600}, m_storage, platform);

  m_command_lists.clear();
  m_command_lists.reserve(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_command_lists.push_back(
        std::make_unique<ssme::d3d12::Dx12CommandList>(*m_device, m_storage));
  }
  /* -------------SETUP 3D-------------- */

  /* -------------SETUP MISC-------------- */
}

//================================================================

void Dx12Renderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  // m_imgui_context = ctx;
  /* -------------INIT UI-------------- */
  m_imgui_context = ctx;

  auto device = m_device->getHandle().Get();
  auto queue = m_device->getCommandQueue().Get();
  auto rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // матчится с PSO

  // создать SRV heap для ImGui (1 дескриптор = шрифт)
  D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
  heap_desc.NumDescriptors = 1;
  heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_imgui_srv_heap));

  m_imgui_srv_handle.cpu =
      m_imgui_srv_heap->GetCPUDescriptorHandleForHeapStart();
  m_imgui_srv_handle.gpu =
      m_imgui_srv_heap->GetGPUDescriptorHandleForHeapStart();

  ImGui_ImplDX12_InitInfo info = {};
  info.Device = device;
  info.CommandQueue = queue;
  info.NumFramesInFlight = MAX_FRAMES_IN_FLIGHT;
  info.RTVFormat = rtv_format;
  info.SrvDescriptorHeap = m_imgui_srv_heap.Get();
  info.SrvDescriptorAllocFn = Dx12ImguiSrvAlloc;
  info.SrvDescriptorFreeFn = Dx12ImguiSrvFree;
  info.UserData = &m_imgui_srv_handle;

  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplDX12_Init(&info);
  m_imgui_initialized = true;
}

// Explicit destructor in .cpp file allows unique_ptr to see full type
// definitions
Dx12Renderer::~Dx12Renderer() { destroy(); };

void Dx12Renderer::beginFrame() {
  m_swap_chain->acquireNextImage();
}

SwapchainInfo Dx12Renderer::getSwapchain() {
  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto extent = m_swap_chain->getSwapChainExtent();
  return {m_swap_chain->getTextureRID(static_cast<uint32_t>(frame_index)),
          extent};
}

uint32_t Dx12Renderer::getCurrentFrameIndex() const {
  return static_cast<uint32_t>(m_swap_chain->getCurrentFrameIndex());
}

void Dx12Renderer::renderImGui(CommandList &cmd, ImDrawData *ui) {
  if (!ui || !m_imgui_initialized)
    return;
  ImGui::SetCurrentContext(m_imgui_context);
  auto &dxcmd = static_cast<ssme::d3d12::Dx12CommandList &>(cmd);
  ID3D12DescriptorHeap *heaps[] = {m_imgui_srv_heap.Get()};
  dxcmd.getCommandListHandle()->SetDescriptorHeaps(1, heaps);
  ImGui_ImplDX12_RenderDrawData(ui, dxcmd.getCommandListHandle().Get());
}

void Dx12Renderer::renderFrameGraph(RenderGraph &graph,
                                    const CompiledPlan &plan,
                                    SceneView &view, ImDrawData *ui) {
  updatePerFrameResources(view);

  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto extent = m_swap_chain->getSwapChainExtent();
  Rect rect{.x = 0, .y = 0, .width = extent.width, .height = extent.height};

  auto *cmd = m_command_lists[frame_index].get();
  cmd->begin();

  graph.execute(*cmd, plan, rect);
  // вернуть транзиентные ресурсы в COMMON — иначе на след. кадре первый
  // барьер (UNDEFINED→X) не совпадёт с реальным состоянием GPU
  graph.resetTransientToCommon(*cmd);
  // финальный переход backbuffer COPY_DEST -> PRESENT (как у Vulkan)
  RID bb = m_swap_chain->getTextureRID(static_cast<uint32_t>(frame_index));
  BarrierInfo to_present;
  to_present.image_barriers.push_back(
      {bb, ImageLayout::TRANSFER_DST, ImageLayout::PRESENT_SRC});
  cmd->pipelineBarrier(to_present);

  cmd->end();
  m_swap_chain->submitCommandBuffers(cmd->getCommandListHandle());
}

void Dx12Renderer::destroy() {
  waitIdle();
  if (m_imgui_initialized) {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplDX12_Shutdown();
    m_imgui_initialized = false;
  }
}

void Dx12Renderer::setFrameResources(std::shared_ptr<FrameData> data) {
  m_frame_data = data;
}

void Dx12Renderer::updatePerFrameResources(const SceneView &view) {
  // Get uniform buffer
  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto &ubo = m_frame_data->uniform_buffer[frame_index];
  // Get viewport dimensions for aspect ratio
  auto extent = m_swap_chain->getSwapChainExtent();
  float aspect_ratio = extent.width / static_cast<float>(extent.height);
  // Calculate view-projection matrix
  // Camera at (0, 0, 5) looking at (0, 0, 0), up is +Y
  glm::mat4 view_mat = glm::lookAt(
      glm::vec3(0.0f + view.x, 0.0f, 5.0f + view.z), // Camera position
      glm::vec3(0.0f, 0.0f, 0.0f),                   // Look at target
      glm::vec3(0.0f, 1.0f, 0.0f)                    // Up direction
  );
  glm::mat4 proj_mat =
      glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);

  Uniforms::FrameUniforms uniforms{};
  uniforms.view_projection = proj_mat * view_mat;
  uniforms.light_position = glm::vec3(0.0f, 0.0f, 1.0f);
  uniforms.Kd =
      glm::vec3(1.0f, 1.0f, 1.0f); // Diffuse coefficient (white surface)
  uniforms.Ld = glm::vec3(1.0f, 1.0f, 1.0f); // Light intensity (white light)
  uniforms.camera_position = glm::vec3(0.0f, 5.0f, 5.0f);
  auto packed = Uniforms::FrameUniformsStd140::from(uniforms);
  // Update buffer
  m_rhi_device->updateBuffer(ubo->getUbo(), packed);
}

void Dx12Renderer::waitIdle() const {
  // m_swap_chain не const, зато m_device const-ref, поэтому:
  const_cast<Dx12Renderer *>(this)->m_swap_chain->waitForGpu();
}

GpuBackend Dx12Renderer::getGpuBackend() { return m_backend_type; }

} // namespace ssme
