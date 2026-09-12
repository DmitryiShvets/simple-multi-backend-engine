#pragma once
#include "core/time.h"
#include "events/event.h"
#include "ui/hello_widget.h"
#include "world/ecs/entity.h"
#include <glm/fwd.hpp>
#include <memory>

// ssme = Simple Study Multi-backend Engine
namespace ssme {

class Scene;
class System;
class Platform;
class UIManager;
class RenderSystem;
class ResourceManager;
class CameraComponent;
class EventBus;
class ActionBus;
class InputSystem;
class Camera;
class CameraController;
class CameraActionHandler;

class Engine : public EventListener {
public:
  /**
   * @brief Default constructor.
   */
  Engine();
  /**
   * @brief Destructor for proper cleanup.
   */
  ~Engine();
  /**
   * @brief Initialize the engine.
   * @param width The width of the window.
   * @param height The height of the window.
   * @return True if initialization was successful, false otherwise.
   */
  bool initialize(int width, int height);
  /**
   * @brief Run the main game loop.
   */
  void run();
  /**
   * @brief Clean up engine resources.
   */
  void cleanup();

  Entity createSphere(const std::string &name, float radius, int stacks,
                      int sectors, const glm::vec3 &pos,
                      const std::string &mat_name = "ads");
  Entity createTriangle(const std::string &name, const glm::vec3 &pos,
                        const std::string &mat_name = "default");
  Entity createQuad(const std::string &name, const glm::vec3 &pos);
  /**
   * @brief Create a new entity.
   * @param name The name of the entity.
   * @return A pointer to the newly created entity.
   */
  Entity *createEntity(const std::string &name);

  /**
   * @brief Get an entity by name.
   * @param name The name of the entity.
   * @return A pointer to the entity, or nullptr if not found.
   */
  Entity *getEntity(const std::string &name);
  /**
   * @brief Get all entities.
   * @return A const reference to the vector of entities.
   */
  const std::vector<std::unique_ptr<Entity>> &getEntities() const {
    return m_entities;
  }

  /**
   * @brief Remove an entity.
   * @param entity The entity to remove.
   * @return True if the entity was removed, false otherwise.
   */
  bool removeEntity(Entity *entity);

  /**
   * @brief Remove an entity by name.
   * @param name The name of the entity to remove.
   * @return True if the entity was removed, false otherwise.
   */
  bool removeEntity(const std::string &name);

  /**
   * @brief Receive events from the EventBus (hotkeys → actions).
   */
  void onEvent(const Event &event) override;

private:
  std::unique_ptr<Platform> m_platform;
  std::unique_ptr<ResourceManager> m_resource_manager;
  std::unique_ptr<UIManager> m_ui_manager;
  std::unique_ptr<RenderSystem> m_render_system;
  std::unique_ptr<Scene> m_scene;

  std::vector<std::unique_ptr<Entity>> m_entities;
  // Active camera
  CameraComponent *m_active_camera = nullptr;

  HelloWidget m_test_widget; // this is templrary

  // Engine state
  bool m_initialized = false;
  bool m_running = false;
  int m_window_width = 0;
  int m_window_height = 0;

  // Delta time calculation
  // deltaTimeMs: time since last frame in milliseconds (for clarity)
  TimeDelta m_delta_time_ms{0};
  uint64_t m_last_frame_time_ms = 0;

  // Frame counter and FPS calculation
  uint64_t m_frame_count = 0;
  float m_fps_update_timer = 0.0f;
  float m_current_fps = 0.0f;
  uint64_t m_last_fps_update_frame = 0;

  // Mouse position tracking
  float m_current_mouse_x = 0.0f;
  float m_current_mouse_y = 0.0f;

  // user input and camera
  std::unique_ptr<EventBus> m_event_bus;
  std::unique_ptr<InputSystem> m_input_system;
  std::unique_ptr<ActionBus> m_action_bus;
  std::vector<std::shared_ptr<Camera>> m_cameras;
  std::vector<std::unique_ptr<CameraController>> m_camera_controllers;
  std::unique_ptr<CameraActionHandler> m_camera_actions;

  /**
   * @brief Update the engine state.
   * @param deltaTime The time elapsed since the last update.
   */
  // Accepts a time delta in milliseconds for clarity
  void update(TimeDelta deltaTime);

  /**
   * @brief Render the scene.
   */
  void render();

  /**
   * @brief Calculate the time delta between frames.
   * @return The delta time in milliseconds (steady_clock based).
   */
  TimeDelta calculateDeltaTimeMs();

  /**
   * @brief Handle window resize events.
   * @param width The new width of the window.
   * @param height The new height of the window.
   */
  void handleResize(int width, int height) const;

  /**
   * @brief Update camera controls based on input state.
   * @param deltaTime The time elapsed since the last update.
   */
  void updateCameraControls(TimeDelta deltaTime);
};

} // namespace ssme
