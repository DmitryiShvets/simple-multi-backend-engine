#pragma once
#include "types.h"

namespace Render {
// Forward-declare
class RenderGraph;
class Device;
// This is the "Graph Executor" (Level 4b).
// It's an API-agnostic class that takes a RenderGraph and executes it
// using the abstract Device interface.
class RenderGraphExecutor {
public:
  RenderGraphExecutor(Device *device);
  virtual ~RenderGraphExecutor();

  // The main function: compiles and executes the frame's graph.
  void execute(RenderGraph &graph, RID backbuffer);

private:
  Device *m_rhi_device;
};

} // namespace Render
