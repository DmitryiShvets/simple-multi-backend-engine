#pragma once

#include "uniform_value.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>

namespace ssme {

/**
 * @brief UniformSet — adapter for working with material uniform data
 *
 * Wrapper over UniformMap with additional conveniences:
 * - Iteration for binary packing
 * - Property presence checking
 * - Convenience methods for frequent types
 *
 * Used during material creation/editing (not per-frame!)
 */
class UniformSet {
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    UniformSet() = default;

    // From initializer list for convenience
    UniformSet(std::initializer_list<std::pair<const std::string, UniformValue>> init)
        : m_values(init) {}

    // From UniformMap (for compatibility)
    explicit UniformSet(UniformMap&& map) : m_values(std::move(map)) {}
    explicit UniformSet(const UniformMap& map) : m_values(map) {}

    // ========================================================================
    // Element Access
    // ========================================================================

    /**
     * @brief Set uniform value by name
     */
    UniformSet& set(const std::string& name, UniformValue value) {
        m_values[name] = std::move(value);
        return *this;
    }

    /**
     * @brief Get uniform value by name (returns nullptr if not found)
     */
    const UniformValue* get(const std::string& name) const {
        auto it = m_values.find(name);
        return it != m_values.end() ? &it->second : nullptr;
    }

    UniformValue* get(const std::string& name) {
        auto it = m_values.find(name);
        return it != m_values.end() ? &it->second : nullptr;
    }

    /**
     * @brief Check if uniform exists
     */
    bool has(const std::string& name) const {
        return m_values.find(name) != m_values.end();
    }

    /**
     * @brief Remove uniform by name
     */
    void remove(const std::string& name) {
        m_values.erase(name);
    }

    // ========================================================================
    // Convenience Setters
    // ========================================================================

    void setVec3(const std::string& name, const glm::vec3& value) {
        set(name, UniformValue(value));
    }

    void setFloat(const std::string& name, float value) {
        set(name, UniformValue(value));
    }

    void setInt(const std::string& name, int32_t value) {
        set(name, UniformValue(value));
    }

    void setBool(const std::string& name, bool value) {
        set(name, UniformValue(value));
    }

    void setMat4(const std::string& name, const glm::mat4& value) {
        set(name, UniformValue(value));
    }

    // ========================================================================
    // Iteration (for packing)
    // ========================================================================

    using const_iterator = UniformMap::const_iterator;
    using iterator = UniformMap::iterator;

    const_iterator begin() const { return m_values.begin(); }
    const_iterator end() const { return m_values.end(); }
    iterator begin() { return m_values.begin(); }
    iterator end() { return m_values.end(); }

    size_t size() const { return m_values.size(); }
    bool empty() const { return m_values.empty(); }

    /**
     * @brief Get underlying map (for compatibility)
     */
    const UniformMap& toMap() const { return m_values; }
    UniformMap& toMap() { return m_values; }

    /**
     * @brief Merge another UniformSet into this one
     */
    void merge(const UniformSet& other) {
        for (const auto& [name, value] : other) {
            m_values[name] = value;
        }
    }

private:
    UniformMap m_values;
};

} // namespace ssme
