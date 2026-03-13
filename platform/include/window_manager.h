#pragma once

#include "i_main_window.h"
#include "backend_type.h"  // ← Core::BackendType
#include <functional>
#include <memory>
#include <vector>

namespace Window {

// Фасад над несколькими окнами
class WindowManager {
public:
  WindowManager() = default;
  WindowManager(const WindowManager &) = delete;
  WindowManager &operator=(const WindowManager &) = delete;

  // Добавление окна в менеджер
  void addWindow(std::unique_ptr<IMainWindow> window);

  // Доступ к окну по типу бекенда
  IMainWindow &getWindow(Core::BackendType type);
  const IMainWindow &getWindow(Core::BackendType type) const;

  // Доступ к окну по индексу (для инициализации)
  IMainWindow &getWindowByIndex(size_t index);
  const IMainWindow &getWindowByIndex(size_t index) const;

  // Количество окон
  size_t getWindowCount() const;

  // Проверка: все ли окна открыты
  bool allAlive() const;

  // Операции над всеми окнами
  void update();
  void swapBuffers();
  void destroy();
  void setPositions(const std::vector<std::pair<int, int>> &positions);
  void setWindowPosition(int index, std::pair<int, int> position);
  void setWindowPosition(Core::BackendType type, std::pair<int, int> position);

  // Итерация для кастомных операций
  template <typename Func> void forEach(Func &&func) {
    for (auto &window : m_windows) {
      func(*window);
    }
  }

  template <typename Func> void forEach(Func &&func) const {
    for (const auto &window : m_windows) {
      func(*window);
    }
  }

private:
  std::vector<std::unique_ptr<IMainWindow>> m_windows;
};

} // namespace Window
