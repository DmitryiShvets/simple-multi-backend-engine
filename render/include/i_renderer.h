#pragma once

// Forward-declare to avoid including heavy headers in the interface
class SceneView;

namespace Render {
// The main, top-level, API-agnostic interface for the entire rendering system.
// The Application will only interact with this interface.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // Renders a single frame based on the provided scene data.
    // This single call encapsulates all work, including acquiring the next image
    // and presenting it at the end.
    virtual void renderFrame(const SceneView& view) = 0;
};
} // namespace Render