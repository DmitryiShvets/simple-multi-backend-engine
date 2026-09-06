#pragma once
#include "core/time.h"
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

class Engine {
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

  // onfy for tests

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
   * @brief Handles mouse input for interaction and camera control.
   *
   * This method processes mouse input for various functionalities, including
   * interacting with the scene, camera rotation, and delegating handling to
   * ImGui or hover systems.
   *
   * @param x The x-coordinate of the mouse position.
   * @param y The y-coordinate of the mouse position.
   * @param buttons A bitmask representing the state of mouse buttons.
   *                Bit 0 corresponds to the left button, and Bit 1 corresponds
   * to the right button.
   */
  void handleMouseInput(float x, float y, uint32_t buttons);

  /**
   * @brief Handles keyboard input events for controlling the camera and other
   * subsystems.
   *
   * This method processes key press and release events to update the camera's
   * movement state. It also forwards the input to other subsystems like the
   * ImGui interface if applicable.
   *
   * @param key The key code of the keyboard input.
   * @param pressed Indicates whether the key is pressed (true) or released
   * (false).
   */
  void handleKeyInput(uint32_t key, bool pressed);

  /**
   * @brief Handle mouse hover to track current mouse position.
   * @param mouseX The x-coordinate of the mouse position.
   * @param mouseY The y-coordinate of the mouse position.
   */
  void handleMouseHover(float mouseX, float mouseY);

  /**
   * @brief Update camera controls based on input state.
   * @param deltaTime The time elapsed since the last update.
   */
  void updateCameraControls(TimeDelta deltaTime);
};

} // namespace ssme
