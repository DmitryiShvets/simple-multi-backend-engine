#pragma once

#include "render_graph.h"

// Forward-declare
class SceneView;

namespace Render {

// This is the "Graph Builder" (Level 4a).
// It's an API-agnostic class whose job is to translate high-level scene data
// into a sequence of render passes (a RenderGraph).
class SceneRenderer {
public:
    SceneRenderer();
    virtual ~SceneRenderer();

    // The main function: takes scene data and returns a "blueprint" for the frame.
    RenderGraph buildGraph(const SceneView& sceneView);
};

} // namespace Render
