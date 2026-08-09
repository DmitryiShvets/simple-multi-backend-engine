#pragma once

#include "core/render_types.h"
#include "render_gtaph_utils.h"
#include <cstdint>
#include <memory>
#include <string>

namespace ssme {

// Forward-declare to avoid circular dependency
class RenderGraph;

// Represents a single step in the render graph (e.g., G-Buffer Pass, Shadow
// Pass)
class RenderPass {
public:
  // Factory method for creation
  static std::unique_ptr<RenderPass> Create(const std::string &name,
                                            uint32_t index, RenderGraph *graph);

  // Declarative API for specifying resource dependencies
  RenderPass &read(ResourceView resource);
  RenderPass &write(ResourceView resource);
  RenderPass &readWrite(ResourceView resource);
  RenderPass &setLoadOp(LoadOp op);
  RenderPass &read(ResourceView resource, ResourceState state);
  RenderPass &write(ResourceView resource, ResourceState state);

  // for compiler
  const std::string &name() const;
  const ExecuteCallback &getExecuteCallback() const;
  void setExecuteCallback(ExecuteCallback &&callback);
  LoadOp getLoadOp() const { return m_load_op; }
  RenderingInfo rendering_info;

private:
  // Private constructor to enforce creation via factory
  RenderPass(const std::string &name, uint32_t index, RenderGraph *graph);

  LoadOp m_load_op = LoadOp::AUTO; // it means AUTO
  StoreOp m_store_op = StoreOp::STORE;

  std::string m_name;
  RenderGraph *m_graph;
  uint32_t m_index;

  ExecuteCallback m_execute_callback;
};

} // namespace ssme
