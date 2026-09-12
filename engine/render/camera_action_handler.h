#pragma once

#include "actions.h"
#include "action_bus.h"
#include "camera_controller.h"

#include <memory>
#include <vector>

namespace ssme {

class CameraActionHandler {
public:
  CameraActionHandler(ActionBus &bus,
                      std::vector<std::unique_ptr<CameraController>> &controllers)
      : m_controllers(controllers) {
    bus.on<ResetViewAction>([this](const ResetViewAction &action) {
      m_controllers[action.window_id]->reset();   // логика — в модуле камеры
    });
  }

private:
  std::vector<std::unique_ptr<CameraController>> &m_controllers;
};

} // namespace ssme
