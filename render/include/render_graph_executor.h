#pragma once
#include "render_types.h"
#include "resource_types.h"

namespace Render {
// Forward-declare
class RenderGraph;
class Device;
class CommandList;

// This is the "Graph Executor" (Level 4b).
// It's an API-agnostic class that takes a RenderGraph and executes it
// by recording commands into a provided CommandList.
class RenderGraphExecutor {
public:
  RenderGraphExecutor(Device *device);
  ~RenderGraphExecutor();

  // The main function: compiles the graph and records its commands into the
  // provided command list.
  void execute(RenderGraph &graph, RID backbuffer, CommandList &cmd, const Rect& render_area);

private:
  Device *m_rhi_device;
};

} // namespace Render
