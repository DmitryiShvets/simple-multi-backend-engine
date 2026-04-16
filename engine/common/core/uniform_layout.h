#pragma once

#include "uniform_set.h"
#include "uniform_value.h"
#include <vector>
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

namespace ssme {

/**
 * @brief UniformLayout — describes uniform block layout for data packing
 *
 * Contains information about offsets, sizes and alignment of each variable.
 * Used for packing UniformSet into binary buffer for GPU.
 *
 * Supports std140 layout (required for Vulkan/OpenGL uniform buffers).
 */
class UniformLayout {
public:
    /**
     * @brief Description of a single variable in layout
     */
    struct Variable {
        std::string name;           ///< Variable name (must match shader!)
        size_t offset;              ///< Offset in bytes from start of buffer
        size_t size;                ///< Size in bytes
        UniformValue::Type type;    ///< Type for validation

        /**
         * @brief Alignment according to std140
         */
        size_t alignment() const;
    };

    /**
     * @brief Add variable to layout
     * @param name Variable name
     * @param type Variable type
     * @param manual_offset Manual offset (0 for auto-calculation by std140)
     */
    UniformLayout& addVariable(const std::string& name,
                                UniformValue::Type type,
                                size_t manual_offset = 0);

    /**
     * @brief Get variable by name
     */
    const Variable* getVariable(const std::string& name) const;

    /**
     * @brief Get all variables
     */
    const std::vector<Variable>& getVariables() const { return m_variables; }

    /**
     * @brief Get total buffer size with alignment
     */
    size_t getTotalSize() const { return m_total_size; }

    /**
     * @brief Calculate size and offsets according to std140
     *
     * Called after adding all variables.
     */
    void computeLayout();

    /**
     * @brief Pack UniformSet into binary buffer
     *
     * @param uniform_set Data to pack
     * @return Binary buffer of getTotalSize()
     *
     * @note Variables are copied at offsets from layout.
     *       Variables without value in uniform_set are zero-filled.
     */
    std::vector<uint8_t> pack(const UniformSet& uniform_set) const;

    /**
     * @brief Pack into existing buffer (no allocations)
     *
     * @param uniform_set Data to pack
     * @param buffer Buffer for writing (must be >= getTotalSize())
     */
    void packTo(const UniformSet& uniform_set, uint8_t* buffer) const;

    /**
     * @brief Create std140 layout from variable list
     *
     * Convenience method for quick layout creation.
     */
    static UniformLayout createStd140(std::initializer_list<Variable> vars);

private:
    /**
     * @brief Get alignment for type according to std140
     */
    static size_t getStd140Alignment(UniformValue::Type type);

    /**
     * @brief Get size for type according to std140
     */
    static size_t getStd140Size(UniformValue::Type type);

    /**
     * @brief Align value up to alignment
     */
    static size_t alignUp(size_t value, size_t alignment);

    std::vector<Variable> m_variables;
    size_t m_total_size = 0;
    bool m_computed = false;
};

// ============================================================================
// Inline Implementation
// ============================================================================

inline size_t UniformLayout::alignUp(size_t value, size_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

} // namespace ssme
