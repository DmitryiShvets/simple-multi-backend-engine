#pragma once

#include "render_pass.h" // Include the new header
#include <memory>
#include <string>
#include <vector>

namespace ssme {

// Forward-declare
class RenderPass;

// A graph structure that defines all the passes for a single frame
class RenderGraph {
public:
  RenderGraph();
  ~RenderGraph();

  // Move-only semantics
  RenderGraph(const RenderGraph &) = delete;
  RenderGraph &operator=(const RenderGraph &) = delete;
  RenderGraph(RenderGraph &&) = default;
  RenderGraph &operator=(RenderGraph &&) = default;

  // Adds a new pass to the graph, returning a reference to it for configuration
  RenderPass &addPass(const std::string &name);

  // Analyzes dependencies and prepares the graph for execution
  void compile();

  // --- Accessors for the Executor ---
  const std::vector<std::unique_ptr<RenderPass>> &getPasses() const {
    return m_passes;
  }

private:
  std::vector<std::unique_ptr<RenderPass>> m_passes;
  bool m_is_compiled = false;
};

} // namespace ssme
