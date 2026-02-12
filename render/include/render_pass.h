#pragma once

#include "command_list.h"
#include "resource_types.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Render {

// Forward-declare to avoid circular dependency
class RenderGraph;

// Represents a single step in the render graph (e.g., G-Buffer Pass, Shadow Pass)
class RenderPass {
public:
  // Factory method for creation
  static std::unique_ptr<RenderPass> Create(const std::string &name,
                                            RenderGraph *graph);

  // Declarative API for specifying resource dependencies
  RenderPass &reads(RID resource);
  RenderPass &writes(RID resource);

  // Sets the actual work to be done in this pass
  void setExecuteCallback(std::function<void(CommandList &cmd)> &&callback);

  // --- Accessors for the Executor ---
  const std::string &name() const { return m_name; }
  const std::vector<RID> &reads() const { return m_reads; }
  const std::vector<RID> &writes() const { return m_writes; }
  const std::function<void(CommandList &cmd)> &
  getExecuteCallback() const {
    return m_execute_callback;
  }

private:
  // Private constructor to enforce creation via factory
  RenderPass(const std::string &name, RenderGraph *graph);

  std::string m_name;
  RenderGraph *m_graph;
  std::vector<RID> m_reads;
  std::vector<RID> m_writes;
  std::function<void(CommandList &cmd)> m_execute_callback;
};

} // namespace Render
