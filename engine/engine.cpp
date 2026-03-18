#include "engine.h"
#include "core/gpu_types.h"
#include "core/scene_view.h"
#include "desktop/desktop_platform.h"
#include "hello_widget.h"
#include "platform.h"
#include "render/render_system.h"
#include "resources/resource_manager.h"
#include "ui_manager.h"

namespace ssme {

Engine::Engine() = default;
Engine::~Engine() { cleanup(); }

bool Engine::initialize(int width, int height) {
  m_window_width = width, m_window_height = height;
  m_platform = std::make_unique<DesktopPlatform>();
  m_platform->initialize("MyApp", width, height); // ✅ Сначала создать окна
  m_platform->addWindow("Opengl Window", width, height,
                        GpuBackend::OpenGL); // ✅ Добавить второе окно
  m_platform->addWindow("Vulkan Window", width, height,
                        GpuBackend::Vulkan); // ✅ Добавить второе окно

  m_platform->setWindowPosition(static_cast<size_t>(GpuBackend::OpenGL),
                                {100, 100});
  m_platform->setWindowPosition(static_cast<size_t>(GpuBackend::Vulkan),
                                {950, 100});
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

  std::vector<std::reference_wrapper<MainWindow>> windows = {
      m_platform->getWindow(static_cast<size_t>(GpuBackend::OpenGL)),
      m_platform->getWindow(static_cast<size_t>(GpuBackend::Vulkan))};
  // Add backends in SAME order as BackendType enum: OpenGL first, Vulkan second
  m_ui_manager = std::make_unique<UIManager>();
  m_ui_manager->addBackend(GpuBackend::OpenGL);
  m_ui_manager->addBackend(GpuBackend::Vulkan);
  m_ui_manager->init(windows);
  // auto m_world = std::make_unique<World<FlecsWorldImpl>>();
  std::vector<ImGuiContext *> ui_contexts = {
      m_ui_manager->getContext(GpuBackend::OpenGL),
      m_ui_manager->getContext(GpuBackend::Vulkan)};

  m_render_system = std::make_unique<RenderSystem>(m_platform.get());
  m_render_system->addBackend(GpuBackend::OpenGL);
  m_render_system->addBackend(GpuBackend::Vulkan);
  m_render_system->init(ui_contexts);

  m_initialized = true;
  return true;
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

    // Clean up subsystems in reverse order of creation
    m_resource_manager.reset();
    m_render_system.reset();
    m_ui_manager.reset();
    m_platform.reset();

    m_initialized = false;
  }
}

void Engine::run() {
  if (!m_initialized) {
    throw std::runtime_error("Engine not initialized");
  }
  m_running = true;

  // Main loop
  while (m_platform->allWindowsAlive()) {
    // Process platform events
    m_platform->updateAllWindows();
    // Render
    render();
    // Process opengl buffers (line swap chain)
    m_platform->swapOpenGLBuffers();
  }
}

void Engine::render() {
  // TODO: ADD SUPPORT FOR USER WIDGETS TO UI MANAGER
  m_ui_manager->render([this]() { m_test_widget.render(); });
  std::vector<SceneView> scenes{SceneView(), SceneView()};
  std::vector<ImDrawData *> ui_draw_bundle = m_ui_manager->getBundleDrawData();
  m_render_system->render(scenes, ui_draw_bundle);
}

// callbacks
void Engine::handleResize(int width, int height) const {}
void Engine::handleMouseInput(float x, float y, uint32_t buttons) {}
void Engine::handleKeyInput(uint32_t key, bool pressed) {}
void Engine::handleMouseHover(float mouseX, float mouseY) {}
} // namespace ssme
