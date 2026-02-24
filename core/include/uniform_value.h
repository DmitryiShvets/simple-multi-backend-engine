#pragma once

#include "rid.h"
#include <variant>
#include <vector>
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

namespace Core {

/**
 * @brief UniformValue — type-safe container for shader uniform values
 *
 * Can only hold types that are supported by GLSL/HLSL shaders.
 * This provides compile-time type safety and efficient storage.
 *
 * Supported types:
 * - Scalars: float, int32_t, uint32_t, bool
 * - Vectors: vec2, vec3, vec4, ivec2/3/4, uvec2/3/4
 * - Matrices: mat2, mat3, mat4
 * - Resources: RID (for textures/samplers)
 * - Arrays: std::vector<float>, std::vector<vec3>, std::vector<vec4>
 */
class UniformValue {
public:
    /**
     * @brief Enum identifying the type stored in UniformValue
     */
    enum class Type {
        // Scalars
        Float,
        Int,
        Uint,
        Bool,
        Double,

        // Vectors (float)
        Vec2, Vec3, Vec4,

        // Vectors (int)
        IVec2, IVec3, IVec4,

        // Vectors (uint)
        UVec2, UVec3, UVec4,

        // Matrices
        Mat2, Mat3, Mat4,

        // Resources
        Texture,  // RID for texture/sampler

        // Arrays
        FloatArray,
        Vec2Array,
        Vec3Array,
        Vec4Array,

        Unknown
    };

    /**
     * @brief Internal variant type holding all possible uniform values
     */
    using ValueType = std::variant<
        // Scalars
        float,
        int32_t,
        uint32_t,
        bool,
        double,

        // Vectors (float)
        glm::vec2, glm::vec3, glm::vec4,

        // Vectors (int)
        glm::ivec2, glm::ivec3, glm::ivec4,

        // Vectors (uint)
        glm::uvec2, glm::uvec3, glm::uvec4,

        // Matrices
        glm::mat2, glm::mat3, glm::mat4,

        // Resources
        RID,

        // Arrays
        std::vector<float>,
        std::vector<glm::vec2>,
        std::vector<glm::vec3>,
        std::vector<glm::vec4>
    >;

    // ========================================================================
    // Constructors
    // ========================================================================

    UniformValue() = default;

    // Scalars
    UniformValue(float v) : value_(v) {}
    UniformValue(int32_t v) : value_(v) {}
    UniformValue(uint32_t v) : value_(v) {}
    UniformValue(bool v) : value_(v) {}
    UniformValue(double v) : value_(v) {}

    // Vectors (float)
    UniformValue(const glm::vec2& v) : value_(v) {}
    UniformValue(const glm::vec3& v) : value_(v) {}
    UniformValue(const glm::vec4& v) : value_(v) {}

    // Vectors (int)
    UniformValue(const glm::ivec2& v) : value_(v) {}
    UniformValue(const glm::ivec3& v) : value_(v) {}
    UniformValue(const glm::ivec4& v) : value_(v) {}

    // Vectors (uint)
    UniformValue(const glm::uvec2& v) : value_(v) {}
    UniformValue(const glm::uvec3& v) : value_(v) {}
    UniformValue(const glm::uvec4& v) : value_(v) {}

    // Matrices
    UniformValue(const glm::mat2& v) : value_(v) {}
    UniformValue(const glm::mat3& v) : value_(v) {}
    UniformValue(const glm::mat4& v) : value_(v) {}

    // Resources
    UniformValue(RID rid) : value_(rid) {}

    // Arrays (template to avoid code duplication)
    template<typename T>
    UniformValue(const std::vector<T>& arr) : value_(arr) {}

    template<typename T>
    UniformValue(std::vector<T>&& arr) : value_(std::move(arr)) {}

    // Initializer list convenience constructors
    UniformValue(std::initializer_list<float> init)
        : value_(std::vector<float>(init)) {}
    UniformValue(std::initializer_list<glm::vec3> init)
        : value_(std::vector<glm::vec3>(init)) {}
    UniformValue(std::initializer_list<glm::vec4> init)
        : value_(std::vector<glm::vec4>(init)) {}

    // ========================================================================
    // Type Information
    // ========================================================================

    /**
     * @brief Get the type of value stored
     */
    Type getType() const {
        return std::visit([](const auto& v) -> Type {
            return getTypeImpl(v);
        }, value_);
    }

    /**
     * @brief Check if value is of specific type
     */
    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(value_);
    }

    /**
     * @brief Check if value is a scalar type
     */
    bool isScalar() const {
        Type t = getType();
        return t == Type::Float || t == Type::Int || t == Type::Uint ||
               t == Type::Bool || t == Type::Double;
    }

    /**
     * @brief Check if value is a vector type
     */
    bool isVector() const {
        Type t = getType();
        return (t >= Type::Vec2 && t <= Type::Vec4) ||
               (t >= Type::IVec2 && t <= Type::IVec4) ||
               (t >= Type::UVec2 && t <= Type::UVec4);
    }

    /**
     * @brief Check if value is a matrix type
     */
    bool isMatrix() const {
        Type t = getType();
        return t == Type::Mat2 || t == Type::Mat3 || t == Type::Mat4;
    }

    /**
     * @brief Check if value is an array type
     */
    bool isArray() const {
        Type t = getType();
        return t == Type::FloatArray || t == Type::Vec2Array ||
               t == Type::Vec3Array || t == Type::Vec4Array;
    }

    /**
     * @brief Check if value is a resource type (texture/sampler)
     */
    bool isResource() const {
        return getType() == Type::Texture;
    }

    // ========================================================================
    // Value Access
    // ========================================================================
    /**
     * @brief Get raw pointer to data
     */
    const void* data() const {
        return std::visit([](const auto& v) -> const void* {
            return static_cast<const void*>(&v);
        }, value_);
    }
    /**
     * @brief Get value as specific type (returns nullptr if wrong type)
     */
    template<typename T>
    const T* get() const {
        return std::get_if<T>(&value_);
    }

    template<typename T>
    T* get() {
        return std::get_if<T>(&value_);
    }

    /**
     * @brief Get value as specific type (throws if wrong type)
     */
    template<typename T>
    T& as() {
        return std::get<T>(value_);
    }

    template<typename T>
    const T& as() const {
        return std::get<T>(value_);
    }

    /**
     * @brief Visit the stored value (for pattern matching)
     */
    template<typename Visitor>
    auto visit(Visitor&& vis) const {
        return std::visit(std::forward<Visitor>(vis), value_);
    }

    template<typename Visitor>
    auto visit(Visitor&& vis) {
        return std::visit(std::forward<Visitor>(vis), value_);
    }

    // ========================================================================
    // Size Information
    // ========================================================================

    /**
     * @brief Get size in bytes of the stored value
     */
    size_t size() const {
        return std::visit([](const auto& v) -> size_t {
            return sizeof(v);
        }, value_);
    }

    /**
     * @brief Get number of elements (for arrays, returns array size)
     */
    size_t elementCount() const {
        return std::visit([](const auto& v) -> size_t {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::vector<float>>) {
                return v.size();
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec2>>) {
                return v.size();
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec3>>) {
                return v.size();
            } else if constexpr (std::is_same_v<T, std::vector<glm::vec4>>) {
                return v.size();
            } else {
                return 1;  // Non-array types have 1 element
            }
        }, value_);
    }

    // ========================================================================
    // Comparison
    // ========================================================================

    bool operator==(const UniformValue& other) const = default;

    // ========================================================================
    // Label (Meta Information)
    // ========================================================================

    /**
     * @brief Set the label/name for this uniform (e.g., "modelMatrix", "albedo")
     */
    void setLabel(const std::string& label) {
        label_ = label;
    }

    /**
     * @brief Get the label/name of this uniform
     */
    const std::string& getLabel() const {
        return label_;
    }

    /**
     * @brief Check if this uniform has a label
     */
    bool hasLabel() const {
        return !label_.empty();
    }

private:
    // Helper to map C++ types to Type enum
    static Type getTypeImpl(const auto& v) {
        using T = std::decay_t<decltype(v)>;

        // Scalars
        if constexpr (std::is_same_v<T, float>) return Type::Float;
        if constexpr (std::is_same_v<T, int32_t>) return Type::Int;
        if constexpr (std::is_same_v<T, uint32_t>) return Type::Uint;
        if constexpr (std::is_same_v<T, bool>) return Type::Bool;
        if constexpr (std::is_same_v<T, double>) return Type::Double;

        // Vectors (float)
        if constexpr (std::is_same_v<T, glm::vec2>) return Type::Vec2;
        if constexpr (std::is_same_v<T, glm::vec3>) return Type::Vec3;
        if constexpr (std::is_same_v<T, glm::vec4>) return Type::Vec4;

        // Vectors (int)
        if constexpr (std::is_same_v<T, glm::ivec2>) return Type::IVec2;
        if constexpr (std::is_same_v<T, glm::ivec3>) return Type::IVec3;
        if constexpr (std::is_same_v<T, glm::ivec4>) return Type::IVec4;

        // Vectors (uint)
        if constexpr (std::is_same_v<T, glm::uvec2>) return Type::UVec2;
        if constexpr (std::is_same_v<T, glm::uvec3>) return Type::UVec3;
        if constexpr (std::is_same_v<T, glm::uvec4>) return Type::UVec4;

        // Matrices
        if constexpr (std::is_same_v<T, glm::mat2>) return Type::Mat2;
        if constexpr (std::is_same_v<T, glm::mat3>) return Type::Mat3;
        if constexpr (std::is_same_v<T, glm::mat4>) return Type::Mat4;

        // Resources
        if constexpr (std::is_same_v<T, RID>) return Type::Texture;

        // Arrays
        if constexpr (std::is_same_v<T, std::vector<float>>) return Type::FloatArray;
        if constexpr (std::is_same_v<T, std::vector<glm::vec2>>) return Type::Vec2Array;
        if constexpr (std::is_same_v<T, std::vector<glm::vec3>>) return Type::Vec3Array;
        if constexpr (std::is_same_v<T, std::vector<glm::vec4>>) return Type::Vec4Array;

        return Type::Unknown;
    }

    ValueType value_;
    std::string label_;
};

/**
 * @brief Convenience type for a map of uniform values
 *
 * Usage:
 *   UniformMap uniforms;
 *   uniforms["albedo"] = glm::vec3(1.0f, 0.0f, 0.0f);
 *   uniforms["roughness"] = 0.5f;
 *   uniforms["modelMatrix"] = glm::mat4(1.0f);
 */
using UniformMap = std::unordered_map<std::string, UniformValue>;

} // namespace Core
