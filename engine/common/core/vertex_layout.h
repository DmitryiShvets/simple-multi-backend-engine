#pragma once
#include "render_types.h"
#include "utils/hash_utils.h"
#include "utils/debug_assert.h"
#include <vector>
#include <string>
#include <cstdint>

namespace ssme {
/**
 * @brief VertexLayout — describes vertex format for vertex buffer
 *
 * Contains information about vertex structure: attributes (location, binding, format, offset)
 * and binding (stride). Used for creating pipeline vertex input state and
 * validating vertex data.
 *
 * Supports:
 * - Multiple vertex bindings (for advanced cases)
 * - Automatic offset calculation
 * - Integration with VertexInputStateDesc
 *
 * Example usage:
 * @code
 * VertexLayout layout;
 * layout.addBinding(0, sizeof(Vertex));
 * layout.addAttribute(0, 0, Format::R32G32B32_SFLOAT, offsetof(Vertex, position));
 * layout.addAttribute(0, 1, Format::R32G32B32_SFLOAT, offsetof(Vertex, normal));
 * @endcode
 */
class VertexLayout {
public:
    /**
     * @brief Description of a single vertex attribute
     */
    struct Attribute {
        uint32_t binding;       ///< Binding index of vertex buffer
        uint32_t location;      ///< Location in shader (layout(location = X))
        Format format;          ///< Data format (R32G32B32_SFLOAT, etc.)
        uint32_t offset;        ///< Offset in bytes from start of vertex buffer
        std::string name;       ///< Attribute name (for debugging/validation)

        bool operator==(const Attribute& other) const {
            return binding == other.binding &&
                   location == other.location &&
                   format == other.format &&
                   offset == other.offset &&
                   name == other.name;
        }

        std::size_t hash() const {
            std::size_t h = 0;
            hash_combine(h, binding, location, static_cast<uint32_t>(format), offset);
            return h;
        }
    };

    /**
     * @brief Vertex buffer binding description
     */
    struct Binding {
        uint32_t binding;       ///< Binding index
        uint32_t stride;        ///< Step between vertices in bytes
        uint32_t instance_step_rate = 0; ///< 0 for per-vertex, >0 for instanced

        bool operator==(const Binding& other) const {
            return binding == other.binding &&
                   stride == other.stride &&
                   instance_step_rate == other.instance_step_rate;
        }

        std::size_t hash() const {
            std::size_t h = 0;
            hash_combine(h,  binding, stride, instance_step_rate);
            return h;
        }
    };

    // ========================================================================
    // Constructors
    // ========================================================================

    VertexLayout() = default;

    // ========================================================================
    // Binding Configuration
    // ========================================================================

    /**
     * @brief Add vertex buffer binding
     * @param binding Binding index (usually 0 for simple cases)
     * @param stride Step between vertices in bytes (sizeof(VertexType))
     * @param instance_step_rate 0 for per-vertex, >0 for instanced drawing
     */
    VertexLayout& addBinding(uint32_t binding, uint32_t stride, uint32_t instance_step_rate = 0) {
        m_bindings.push_back({binding, stride, instance_step_rate});
        return *this;
    }

    /**
     * @brief Set binding for simple case (single binding with index 0)
     * @param stride Step between vertices in bytes
     */
    VertexLayout& setSingleBinding(uint32_t stride) {
        m_bindings.clear();
        m_bindings.push_back({0, stride, 0});
        return *this;
    }

    // ========================================================================
    // Attribute Configuration
    // ========================================================================

    /**
     * @brief Add vertex attribute
     * @param location Location in shader
     * @param binding Binding index of vertex buffer
     * @param format Data format
     * @param offset Offset in bytes from start of vertex buffer
     * @param name Attribute name (for debugging)
     */
    VertexLayout& addAttribute(uint32_t binding, uint32_t location,
                                Format format, uint32_t offset,
                                const std::string& name = "") {
        m_attributes.push_back({binding, location, format, offset, name});
        return *this;
    }

    /**
     * @brief Add position attribute (vec3)
     */
    VertexLayout& addPosition(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32B32_SFLOAT, offset, "position");
    }

    /**
     * @brief Add normal attribute (vec3)
     */
    VertexLayout& addNormal(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32B32_SFLOAT, offset, "normal");
    }

    /**
     * @brief Add texture coordinate attribute (vec2)
     */
    VertexLayout& addTexCoord(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32_SFLOAT, offset, "tex_coord");
    }

    /**
     * @brief Add color attribute (vec4)
     */
    VertexLayout& addColor(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32B32_SFLOAT, offset, "color");
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    /**
     * @brief Get all bindings
     */
    const std::vector<Binding>& getBindings() const { return m_bindings; }

    /**
     * @brief Get all attributes
     */
    const std::vector<Attribute>& getAttributes() const { return m_attributes; }

    /**
     * @brief Get stride for binding
     */
    uint32_t getStride(uint32_t binding = 0) const {
        for (const auto& b : m_bindings) {
            if (b.binding == binding) {
                return b.stride;
            }
        }
        return 0;
    }

    /**
     * @brief Get attribute count
     */
    size_t getAttributeCount() const { return m_attributes.size(); }

    /**
     * @brief Get binding count
     */
    size_t getBindingCount() const { return m_bindings.size(); }

    // Check compatibility with shader requirements
    bool isCompatibleWith(const std::vector<VertexInputRequirement>& requirements) const {
      // 1. Check that all required locations exist in layout
      for (const auto& req : requirements) {
        bool found = false;
        for (const auto& attr : m_attributes) {
          if (attr.location == req.location) {
            found = true;

            // 2. Check format compatibility
            if (!isFormatCompatible(attr.format, req.expected_format)) {
              debug_assert(false,
                "Format mismatch at location " + std::to_string(req.location));
              return false;
            }
            break;
          }
        }

        if (!found) {
          debug_assert(false,
            "Missing attribute at location " + std::to_string(req.location) +
            " (" + req.name + ") in VertexLayout");
          return false;
        }
      }
      return true;
    }

    bool operator==(const VertexLayout& other) const {
        return m_bindings == other.m_bindings &&
               m_attributes == other.m_attributes;
    }

    std::size_t hash() const {
        std::size_t h = 0;
        // Hash bindings
        for (const auto& binding : m_bindings) {
            hash_combine(h, binding.hash());
        }
        // Hash attributes
        for (const auto& attr : m_attributes) {
            hash_combine(h, attr.hash());
       }
        return h;
    }
private:

    bool isFormatCompatible(Format mesh_format, Format shader_format) const {
    // Exact match
    if (mesh_format == shader_format) {
        return true;
    }

    // Allow some compatible formats
    // For example, vec3 can come as vec4 (with waste)
    // But this depends on your system

    return false;
    }

    std::vector<Binding> m_bindings;
    std::vector<Attribute> m_attributes;
};
} // namespace ssme
