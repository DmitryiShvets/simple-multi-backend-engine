#pragma once

#include <cstddef>

namespace ssme {

struct ResetViewAction {
  size_t window_id;
  using ResultType = void;
};
struct DemoIncrementAction {
  int step;
  using ResultType = int;
};

} // namespace ssme
