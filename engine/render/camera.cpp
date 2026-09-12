#include "camera.h"

#include <cassert>
#include <limits>

#include <glm/gtc/matrix_transform.hpp>

namespace ssme {

void Camera::setOrthographicProjection(float left, float right, float top,
                                       float bottom, float near, float far) {
  projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near,
                                      float far) {
  assert(aspect > std::numeric_limits<float>::epsilon());
  projectionMatrix = glm::perspective(fovy, aspect, near, far);
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction,
                              glm::vec3 up) {
  viewMatrix = glm::lookAt(position, position + direction, up);
  this->position = position;
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
  viewMatrix = glm::lookAt(position, target, up);
  this->position = position;
}
} // namespace ssme
