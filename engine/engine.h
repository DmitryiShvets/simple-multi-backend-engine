#pragma once
#include "core/time.h"
#include "world/ecs/entity.h"
#include <glm/fwd.hpp>
// ssme = Simple Study Multi-backend Engine
namespace ssme {

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

private:
  std::vector<std::unique_ptr<Entity>> m_entities;
  // Active camera
  CameraComponent *m_active_camera = nullptr;

  // Engine state
  bool initialized = false;
  bool running = false;

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
   * @brief Update camera controls based on input state.
   * @param deltaTime The time elapsed since the last update.
   */
  void updateCameraControls(TimeDelta deltaTime);

  /**
   * @brief Handle mouse hover to track current mouse position.
   * @param mouseX The x-coordinate of the mouse position.
   * @param mouseY The y-coordinate of the mouse position.
   */
  void HandleMouseHover(float mouseX, float mouseY);
};

} // namespace ssme
