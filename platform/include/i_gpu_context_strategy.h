#pragma once

namespace Window {

class IGpuContextStrategy {
public:
  virtual ~IGpuContextStrategy() = default;

  virtual void prepareWindowCreationHints() const = 0;
  virtual bool createContext(void *window) const = 0;
};

} // namespace Window
