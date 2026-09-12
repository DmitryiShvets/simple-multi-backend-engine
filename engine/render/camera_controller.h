#pragma once

#include "camera.h"
#include "core/time.h"

#include <cstddef>

namespace ssme {

class InputSystem;

class CameraController {
public:
  CameraController(InputSystem &input, Camera &camera, std::size_t window_id);

  void update(TimeDelta dt, float aspect);
  void reset();
  void setMoveSpeed(float speed) { m_move_speed = speed; }
  void setLookSpeed(float speed) { m_look_speed = speed; }
  float getMoveSpeed() const { return m_move_speed; }

  void setPosition(const glm::vec3 &pos) { m_position = pos; }
  glm::vec3 getPosition() const { return m_position; }

private:
  InputSystem &m_input;
  Camera &m_camera;
  std::size_t m_window_id;

  float m_move_speed = 5.0f;
  float m_look_speed = 0.01f;
  glm::vec3 m_position{0.0f, 0.0f, 5.0f};
  float m_yaw = 0.0f;   // поворот вокруг Y
  float m_pitch = 0.0f; // наклон вверх/вниз
};

} // namespace ssme
