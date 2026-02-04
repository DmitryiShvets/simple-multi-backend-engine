#pragma once

#include "command_list.h"
#include "types.h" // Added to define TextureDesc and other types
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Render {

// Forward-declare to avoid circular dependency
class RenderGraph;

// Represents a single step in the render graph (e.g., G-Buffer Pass, Shadow Pass)
class RenderPass {
public:
    // Declarative API for specifying resource dependencies
    RenderPass& reads(RID resource);
    RenderPass& writes(RID resource);

    // Sets the actual work to be done in this pass
    void setExecuteCallback(std::function<void(CommandList& cmd)>&& callback);

private:
    friend class RenderGraph;
    RenderPass(const std::string& name, RenderGraph* graph);

    std::string m_name;
    RenderGraph* m_graph;
    std::vector<RID> m_reads;
    std::vector<RID> m_writes;
    std::function<void(CommandList& cmd)> m_execute_callback;
};

// A graph structure that defines all the passes for a single frame
class RenderGraph {
public:
    RenderGraph();
    ~RenderGraph();

    // Move-only semantics
    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;
    RenderGraph(RenderGraph&&) = default;
    RenderGraph& operator=(RenderGraph&&) = default;

    // Adds a new pass to the graph, returning a reference to it for configuration
    RenderPass& addPass(const std::string& name);

    // Creates a virtual resource within the graph
    RID createTexture(const std::string& name, const TextureDesc& desc);

    // TODO: Add methods for compiling the graph and executing it
    void compile();
    void execute();

private:
    std::vector<std::unique_ptr<RenderPass>> m_passes;
    // ... internal resource registry and dependency tracking ...
};

} // namespace Render
