#include "camera_controller.h"
#include "input_system.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


#include <cmath>

namespace {
constexpr float kPitchLimit = glm::half_pi<float>() - 0.1f;
constexpr float kNear = 0.1f;
constexpr float kFar = 100.0f;
constexpr float kFovy = glm::radians(60.0f);
} // namespace

namespace ssme {

CameraController::CameraController(InputSystem &input, Camera &camera,
                                   std::size_t window_id)
    : m_input(input), m_camera(camera), m_window_id(window_id) {}

void CameraController::update(TimeDelta dt, float aspect) {
  const InputState &state = m_input.getState(m_window_id);
  const float seconds = static_cast<float>(dt.count()) / 1000.0f;

  // --- Обзор: только при зажатой ПКМ ---
  if (m_input.isMouseDown(m_window_id, MouseButton::Right)) {
    m_yaw += static_cast<float>(state.mouse_dx) * m_look_speed;
    m_pitch -= static_cast<float>(state.mouse_dy) * m_look_speed;
    m_pitch = glm::clamp(m_pitch, -kPitchLimit, kPitchLimit);
  }
  // --- WASD/стрелки: движение в плоскости XZ относительно взгляда ---
  const glm::vec3 forward_dir{ std::sin(m_yaw), 0.0f, -std::cos(m_yaw) };
  const glm::vec3 right_dir{ std::cos(m_yaw), 0.0f, std::sin(m_yaw) };
  const glm::vec3 up_dir{0.0f, 1.0f, 0.0f};

  float move_forward = 0.0f;
  float move_right = 0.0f;
  float move_up = 0.0f;

  if (m_input.isKeyDown(m_window_id, Key::W) ||
      m_input.isKeyDown(m_window_id, Key::ArrowUp)) {
    move_forward += 1.0f;
  }
  if (m_input.isKeyDown(m_window_id, Key::S) ||
      m_input.isKeyDown(m_window_id, Key::ArrowDown)) {
    move_forward -= 1.0f;
  }
  if (m_input.isKeyDown(m_window_id, Key::D) ||
      m_input.isKeyDown(m_window_id, Key::ArrowRight)) {
    move_right += 1.0f;
  }
  if (m_input.isKeyDown(m_window_id, Key::A) ||
      m_input.isKeyDown(m_window_id, Key::ArrowLeft)) {
    move_right -= 1.0f;
  }
  if (m_input.isKeyDown(m_window_id, Key::Space)) {
    move_up += 1.0f;
  }
  if (m_input.isKeyDown(m_window_id, Key::LeftShift)) {
    move_up -= 1.0f;
  }

  glm::vec3 move = forward_dir * move_forward + right_dir * move_right +
                   up_dir * move_up;
  if (glm::length(move) > 1e-6f) {
    move = glm::normalize(move) * m_move_speed * seconds;
  }
  m_position += move;

  const glm::vec3 view_dir{
      std::cos(m_pitch) * std::sin(m_yaw), std::sin(m_pitch),
      -std::cos(m_pitch) * std::cos(m_yaw)};
  m_camera.setViewTarget(m_position, m_position + view_dir);
  m_camera.setPerspectiveProjection(kFovy, aspect, kNear, kFar);
}
void CameraController::reset() {
  m_position = glm::vec3{0.0f, 0.0f, 5.0f};
  m_yaw = 0.0f;
  m_pitch = 0.0f;
}
} // namespace ssme
