#pragma once
#include "engine.h"
#include <memory>

class Application {
public:
  void initialize();
  void run();
  void cleanup();

  Application() = default;
  ~Application();
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

private:
  std::unique_ptr<ssme::Engine> m_engine;
};
