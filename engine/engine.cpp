#include "engine.h"
#include "core/gpu_types.h"
#include "desktop/desktop_platform.h"
#include "ecs/entity.h"
#include "ecs/world.h"
#include "hello_widget.h"
#include "loaders/material_loader.h"
#include "loaders/shader_loader.h"
#include "objects/objects_utils.h"
#include "platform.h"
#include "render/render_system.h"
#include "render_proxy.h"
#include "resource_manager.h"
#include "scene.h"
#include "scene_view.h"
#include "ui_manager.h"
#include "utils/common_utils.h"
#include <flecs.h>
#include <malloc.h>
#include <memory>
namespace ssme {

static constexpr struct {
  GpuBackend backend;
  const char *title;
  int x, y;
  // NOTE: IF g_enable_validation_layers IN DX12 RENDER IS ENABLED, THE BACKEND
  //       INIT ORDER MATTERS: DX12 MUST BE CREATED BEFORE VULKAN.
  //
  // WHY: When D3D12 validation is enabled, Dx12Device::initialize() calls:
  //   D3D12GetDebugInterface → EnableDebugLayer() → CreateDXGIFactory2(DEBUG)
  //
  // EnableDebugLayer() installs a GLOBAL DXGI hook (ID3D12Debug intercepts all
  // IDXGIFactory creation). Later, when Vulkan initializes, glfwCreateWindowSurface
  // internally calls vkCreateWin32SurfaceKHR, which uses DXGI to enumerate
  // adapters and create the surface.
  //
  // If D3D12 debug hook is already active, it wraps any subsequent IDXGIFactory
  // creation. Vulkan's internal CreateDXGIFactory call gets the debug-wrapped
  // factory, and everything works transparently — the debug layer is compatible
  // with DXGI calls from any API.
  //
  // If Vulkan initializes FIRST, it creates its IDXGIFactory WITHOUT the D3D12
  // debug wrapper. When D3D12 validation then enables EnableDebugLayer(), the
  // newly installed debug hook may not properly track Vulkan's already-created
  // DXGI objects. This can cause DXGI internal state mismatch, leading to:
  //   - vkCreateWin32SurfaceKHR failing (ErrorDeviceLost) (I GOT IT)
  //   - IDXGIFactory::EnumAdapters returning unexpected results (I GOT IT)
  //   - D3D12 reporting false-positive CORRUPTION warnings on shared DXGI objects
  //
  // This is a known Windows/DXGI limitation. The D3D12 debug layer and Vulkan WSI
  // share DXGI internally, and the debug hook must be established before any
  // DXGI factories are created — regardless of which API creates them.
} kBackends[] = {
    {GpuBackend::OpenGL, "OpenGL Window", 100, 100},
    {GpuBackend::DirectX12, "DirectX Window", 500, 400},
    {GpuBackend::Vulkan, "Vulkan Window", 950, 100},
};

struct TestComp {
  int x;
};
struct TestComp1 {
  int x;
};

Engine::Engine() = default;
Engine::~Engine() { cleanup(); }

bool Engine::initialize(int width, int height) {
  m_window_width = width, m_window_height = height;
  m_platform = std::make_unique<DesktopPlatform>();
  m_platform->initialize("MyApp", width, height);
   // Register windows in app
  for (auto &b : kBackends) {
    m_platform->addWindow(b.title, width, height, b.backend);
    m_platform->setWindowPosition(b.backend, {b.x, b.y});
  }
  // Set resize callback
  m_platform->setResizeCallback(
      [this](size_t window_index, int width, int height) {
        handleResize(width, height);
      });
  // Set mouse callback
  m_platform->setMouseCallback(
      [this](size_t window_index, float x, float y, uint32_t buttons) {
        handleMouseInput(x, y, buttons);
      });
  // Set keyboard callback
  m_platform->setKeyboardCallback(
      [this](size_t window_index, uint32_t key, bool pressed) {
        handleKeyInput(key, pressed);
      });
  // Set char callback
  m_platform->setCharCallback([this](size_t window_index, uint32_t c) {
    // if (imguiSystem) {
    //   imguiSystem->HandleChar(c);
    // }
  });
  m_ui_manager = std::make_unique<UIManager>();
  m_resource_manager = std::make_unique<ResourceManager>();
  m_render_system = std::make_unique<RenderSystem>(m_platform.get(),
                                                   m_resource_manager.get());
  std::vector<std::reference_wrapper<MainWindow>> windows;
  std::vector<ImGuiContext*> ui_contexts;

  m_resource_manager->registerLoader(std::make_unique<ShaderLoader>());
  m_resource_manager->registerLoader(std::make_unique<MaterialLoader>());

  for (auto& b : kBackends) {
      windows.push_back(m_platform->getWindow(b.backend));
      m_ui_manager->addBackend(b.backend);
  }
  m_ui_manager->init(windows);

  for (auto& b : kBackends) {
      ui_contexts.push_back(m_ui_manager->getContext(b.backend));
      m_render_system->addBackend(b.backend);
  }
  m_render_system->init(ui_contexts);

  // Register render devices with the resource manager
  for (auto& b : kBackends) {
      m_resource_manager->registerDevice(b.backend, &m_render_system->getDevice(b.backend));
  }

  m_render_system->createPerFrameResources();

  m_scene = std::make_unique<Scene>(*m_resource_manager.get());
  m_initialized = true;
  return m_initialized;
}

void Engine::cleanup() {
  if (m_initialized) {
    // Wait for the device to be idle before cleaning up
    if (m_render_system) {
      m_render_system->waitIdleAll();
    }

    // Clear entities
    {
      // std::unique_lock<std::shared_mutex> lk(entitiesMutex);
      m_entities.clear();
    }

    if (m_resource_manager) {
      m_resource_manager->clear();
    }
    // Clean up subsystems in reverse order of creation
    m_scene.reset();
    m_render_system.reset();

    m_resource_manager.reset();
    m_platform->destroy(); // destroy windows backends
    m_ui_manager.reset();  // destroy ui context
    m_platform.reset();    // destroy windows context

    m_initialized = false;
  }
}

void Engine::run() {
  if (!m_initialized) {
    throw std::runtime_error("Engine not initialized");
  }
  m_running = true;
  std::cout << "Flecs version: " << FLECS_VERSION_MAJOR << "."
            << FLECS_VERSION_MINOR << "." << FLECS_VERSION_PATCH << std::endl;

  // Main loop
  while (m_platform->allWindowsAlive()) {
    auto dt = calculateDeltaTimeMs();
    // Process platform events
    m_platform->updateAllWindows();
    // Process Systems
    update(dt);
    // Render
    render();
    // Process opengl buffers (line swap chain)
    m_platform->swapOpenGLBuffers();
  }
}

void Engine::update(TimeDelta deltaTime) { m_scene->update(deltaTime); }

void Engine::render() {
  // TODO: ADD SUPPORT FOR USER WIDGETS TO UI MANAGER
  m_ui_manager->render([this]() { m_test_widget.render(); });

  std::vector<SceneView> scenes = m_scene->getSceneViews(std::size(kBackends));
  for (size_t i = 0; i < std::size(kBackends); i++) {
      scenes[i].x = CUtils::lerp(-3, 3, m_test_widget.m_slider_value);
  }

  std::vector<ImDrawData *> ui_draw_bundle = m_ui_manager->getBundleDrawData();
  m_render_system->render(scenes, ui_draw_bundle);
}

// callbacks
void Engine::handleResize(int width, int height) const {}
void Engine::handleMouseInput(float x, float y, uint32_t buttons) {}
void Engine::handleKeyInput(uint32_t key, bool pressed) {}

// ============================================================================
// Private Methods
// ============================================================================

TimeDelta Engine::calculateDeltaTimeMs() {
  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch())
                    .count();

  if (m_last_frame_time_ms == 0) {
    m_last_frame_time_ms = now_ms;
    return TimeDelta{0};
  }

  m_delta_time_ms = TimeDelta{now_ms - m_last_frame_time_ms};
  m_last_frame_time_ms = now_ms;

  // FPS calculation
  m_frame_count++;
  if (m_frame_count - m_last_fps_update_frame >= 60) {
    m_current_fps = 1000.0f / m_delta_time_ms.count();
    m_last_fps_update_frame = m_frame_count;
  }

  return m_delta_time_ms;
}
void Engine::handleMouseHover(float mouseX, float mouseY) {}

Entity Engine::createSphere(const std::string &name, float radius, int stacks,
                            int sectors, const glm::vec3 &pos,
                            const std::string &mat_name) {
  // TODO: FIX IT scene must create obj
  return _createSphere(m_scene.get()->getWorld(), *m_resource_manager, name,
                       radius, stacks, sectors, pos, mat_name);
}
Entity Engine::createTriangle(const std::string &name, const glm::vec3 &pos,
                              const std::string &mat_name) {
  // TODO: FIX IT scene must create obj
  return _createTriangle(m_scene.get()->getWorld(), *m_resource_manager, name,
                         pos, mat_name);
}
} // namespace ssme
