#pragma once

#include "i_gpu_context_strategy.h"

#include <functional>
#include <string>

namespace Window {

enum class Key {
  Escape,
  Enter,
  Space,
  Num1,
  Num2,
  Num3,
  Num4,
  // ... остальные клавиши
};

enum class MouseButton { Left, Right, Middle };

enum class Action { Press, Release, Repeat };

struct WindowConfig {
  std::string title;
  int width;
  int height;
};

class IMainWindow {
public:
  using MouseCallback =
      std::function<void(MouseButton, Action, int, double, double)>;
  using KeyCallback = std::function<void(Key, Action, int)>;
  using CursorCallback = std::function<void(int, int)>;
  using ResizeCallback = std::function<void(int, int)>;
  using ScrollCallback = std::function<void(double, double)>;
  using CharCallback = std::function<void(unsigned int)>;
  using WindowFocusCallback = std::function<void(bool)>;
  using CursorEnterCallback = std::function<void(bool)>;

  virtual ~IMainWindow() = default;
  virtual void init(const IGpuContextStrategy &contextStrategy) = 0;
  virtual void destroy() = 0;
  virtual bool shouldClose() const = 0;
  virtual void swapBuffers() = 0;
  virtual void update() = 0;
  virtual void setPosition(int x, int y) = 0;
  virtual void setMouseCallback(MouseCallback callback) = 0;
  virtual void setKeyCallback(KeyCallback callback) = 0;
  virtual void setCursorCallback(CursorCallback callback) = 0;
  virtual void setResizeCallback(ResizeCallback callback) = 0;
  virtual void setScrollCallback(ScrollCallback callback) = 0;
  virtual void setCharCallback(CharCallback callback) = 0;
  virtual void setWindowFocusCallback(WindowFocusCallback callback) = 0;
  virtual void setCursorEnterCallback(CursorEnterCallback callback) = 0;

  virtual WindowConfig getConfig() = 0;
  virtual void *getNativeWindow() const = 0;
  virtual void setUiContext(void *ctx) = 0;
};
} // namespace Window
