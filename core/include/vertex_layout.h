#pragma once
#include "common.h"

#include <vector>
#include <string>
#include <cstdint>

namespace Core {
/**
 * @brief VertexLayout — описывает формат вершины для vertex buffer
 *
 * Содержит информацию о структуре вершины: атрибуты (location, binding, format, offset)
 * и binding (stride). Используется для создания vertex input state pipeline и
 * валидации вершинных данных.
 *
 * Поддерживает:
 * - Multiple vertex bindings (для advanced случаев)
 * - Автоматический расчёт оффсетов
 * - Интеграцию с Render::VertexInputStateDesc
 *
 * Пример использования:
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
     * @brief Описание одного атрибута вершины
     */
    struct Attribute {
        uint32_t binding;       ///< Binding index vertex buffer
        uint32_t location;      ///< Location в шейдере (layout(location = X))
        Format format;          ///< Формат данных (R32G32B32_SFLOAT, etc.)
        uint32_t offset;        ///< Оффсет в байтах от начала vertex buffer
        std::string name;       ///< Имя атрибута (для отладки/валидации)
    };

    /**
     * @brief Описание vertex buffer binding
     */
    struct Binding {
        uint32_t binding;       ///< Binding index
        uint32_t stride;        ///< Шаг между вершинами в байтах
        uint32_t instance_step_rate = 0; ///< 0 для per-vertex, >0 для instanced
    };

    // ========================================================================
    // Constructors
    // ========================================================================

    VertexLayout() = default;

    // ========================================================================
    // Binding Configuration
    // ========================================================================

    /**
     * @brief Добавить vertex buffer binding
     * @param binding Binding index (обычно 0 для простых случаев)
     * @param stride Шаг между вершинами в байтах (sizeof(VertexType))
     * @param instance_step_rate 0 для per-vertex, >0 для instanced drawing
     */
    VertexLayout& addBinding(uint32_t binding, uint32_t stride, uint32_t instance_step_rate = 0) {
        m_bindings.push_back({binding, stride, instance_step_rate});
        return *this;
    }

    /**
     * @brief Установить binding для простого случая (один binding с index 0)
     * @param stride Шаг между вершинами в байтах
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
     * @brief Добавить атрибут вершины
     * @param location Location в шейдере
     * @param binding Binding index vertex buffer
     * @param format Формат данных
     * @param offset Оффсет в байтах от начала vertex buffer
     * @param name Имя атрибута (для отладки)
     */
    VertexLayout& addAttribute(uint32_t binding, uint32_t location,
                                Format format, uint32_t offset,
                                const std::string& name = "") {
        m_attributes.push_back({binding, location, format, offset, name});
        return *this;
    }

    /**
     * @brief Добавить атрибут позиции (vec3)
     */
    VertexLayout& addPosition(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32B32_SFLOAT, offset, "position");
    }

    /**
     * @brief Добавить атрибут нормали (vec3)
     */
    VertexLayout& addNormal(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32B32_SFLOAT, offset, "normal");
    }

    /**
     * @brief Добавить атрибут текстуры (vec2)
     */
    VertexLayout& addTexCoord(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32_SFLOAT, offset, "tex_coord");
    }

    /**
     * @brief Добавить атрибут цвета (vec4)
     */
    VertexLayout& addColor(uint32_t binding, uint32_t location, uint32_t offset) {
        return addAttribute(binding, location, Format::R32G32B32_SFLOAT, offset, "color");
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    /**
     * @brief Получить все bindings
     */
    const std::vector<Binding>& getBindings() const { return m_bindings; }

    /**
     * @brief Получить все attributes
     */
    const std::vector<Attribute>& getAttributes() const { return m_attributes; }

    /**
     * @brief Получить stride для binding
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
     * @brief Получить количество атрибутов
     */
    size_t getAttributeCount() const { return m_attributes.size(); }

    /**
     * @brief Получить количество bindings
     */
    size_t getBindingCount() const { return m_bindings.size(); }

private:
    std::vector<Binding> m_bindings;
    std::vector<Attribute> m_attributes;
};

// ============================================================================
// Inline Implementation
// ============================================================================

// inline VertexInputStateDesc VertexLayout::toVertexInputStateDesc() const {
//     VertexInputStateDesc desc;

//     // Convert bindings
//     for (const auto& binding : m_bindings) {
//         desc.bindings.push_back({
//             .binding = binding.binding,
//             .stride = binding.stride
//         });
//     }

//     // Convert attributes
//     for (const auto& attr : m_attributes) {
//         desc.attributes.push_back({
//             .location = attr.location,
//             .binding = attr.binding,
//             .format = attr.format,
//             .offset = attr.offset
//         });
//     }

//     return desc;
// }

} // namespace Core
