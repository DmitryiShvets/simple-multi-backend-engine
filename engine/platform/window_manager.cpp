#include "window_manager.h"
#include "backend_type.h"
#include <stdexcept>

namespace Window {

void WindowManager::addWindow(std::unique_ptr<IMainWindow> window) {
  m_windows.push_back(std::move(window));
}

IMainWindow &WindowManager::getWindow(Core::BackendType type) {
  const size_t index = static_cast<size_t>(type);
  if (index >= m_windows.size()) {
    throw std::out_of_range("Window index out of range");
  }
  return *m_windows[index];
}

const IMainWindow &WindowManager::getWindow(Core::BackendType type) const {
  const size_t index = static_cast<size_t>(type);
  if (index >= m_windows.size()) {
    throw std::out_of_range("Window index out of range");
  }
  return *m_windows[index];
}

IMainWindow &WindowManager::getWindowByIndex(size_t index) {
  if (index >= m_windows.size()) {
    throw std::out_of_range("Window index out of range");
  }
  return *m_windows[index];
}

const IMainWindow &WindowManager::getWindowByIndex(size_t index) const {
  if (index >= m_windows.size()) {
    throw std::out_of_range("Window index out of range");
  }
  return *m_windows[index];
}

size_t WindowManager::getWindowCount() const { return m_windows.size(); }

bool WindowManager::allAlive() const {
  for (const auto &window : m_windows) {
    if (window->shouldClose()) {
      return false;
    }
  }
  return !m_windows.empty();
}

void WindowManager::update() {
  for (auto &window : m_windows) {
    window->update();
  }
}

void WindowManager::swapBuffers() {
  m_windows[0]->swapBuffers();
  // for (auto& window : m_windows) {
  //     window->swapBuffers();
  // }
}

void WindowManager::destroy() {
  for (auto &window : m_windows) {
    window->destroy();
  }
}

void WindowManager::setPositions(
    const std::vector<std::pair<int, int>> &positions) {
  if (positions.size() > m_windows.size()) {
    throw std::invalid_argument("Too many positions provided");
  }
  for (size_t i = 0; i < positions.size(); ++i) {
    m_windows[i]->setPosition(positions[i].first, positions[i].second);
  }
}

void WindowManager::setWindowPosition(Core::BackendType type,
                                      std::pair<int, int> position) {
  const size_t index = static_cast<size_t>(type);
  m_windows[index]->setPosition(position.first, position.second);
}
void WindowManager::setWindowPosition(int index, std::pair<int, int> position) {
  m_windows[index]->setPosition(position.first, position.second);
}

} // namespace Window
