#pragma once

namespace ssme {

/**
 * @brief Strategy pattern for GPU context creation.
 */
class GpuContextStrategy {
public:
    virtual ~GpuContextStrategy() = default;

    /**
     * @brief Prepare window creation hints for this GPU context.
     */
    virtual void prepareWindowCreationHints() const = 0;
    
    /**
     * @brief Create GPU context for the given window.
     * @param window Native window handle (GLFWwindow*).
     * @return True if context was created successfully.
     */
    virtual bool createContext(void* window) const = 0;
};

} // namespace ssme
