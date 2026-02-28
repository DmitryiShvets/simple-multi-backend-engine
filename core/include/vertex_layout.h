#pragma once
#include "common.h"

#include <vector>
#include <string>
#include <cstdint>

namespace Core {
    struct VertexInputBindingDesc {
      uint32_t binding;
      uint32_t stride;
    };

    struct VertexInputAttributeDesc {
      uint32_t location;
      uint32_t binding;
      Format format;
      uint32_t offset;
    };

    struct VertexInputStateDesc {
      std::vector<VertexInputBindingDesc> bindings;
      std::vector<VertexInputAttributeDesc> attributes;
    };
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
 * layout.addAttribute(1, 0, Format::R32G32B32_SFLOAT, offsetof(Vertex, normal));
 * @endcode
 */
class VertexLayout {
public:
    /**
     * @brief Описание одного атрибута вершины
     */
    struct Attribute {
        uint32_t location;      ///< Location в шейдере (layout(location = X))
        uint32_t binding;       ///< Binding index vertex buffer
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
    VertexLayout& addAttribute(uint32_t location, uint32_t binding,
                                Format format, uint32_t offset,
                                const std::string& name = "") {
        m_attributes.push_back({location, binding, format, offset, name});
        return *this;
    }

    /**
     * @brief Добавить атрибут позиции (vec3)
     */
    VertexLayout& addPosition(uint32_t location, uint32_t binding, uint32_t offset) {
        return addAttribute(location, binding, Format::R32G32B32_SFLOAT, offset, "position");
    }

    /**
     * @brief Добавить атрибут нормали (vec3)
     */
    VertexLayout& addNormal(uint32_t location, uint32_t binding, uint32_t offset) {
        return addAttribute(location, binding, Format::R32G32B32_SFLOAT, offset, "normal");
    }

    /**
     * @brief Добавить атрибут текстуры (vec2)
     */
    VertexLayout& addTexCoord(uint32_t location, uint32_t binding, uint32_t offset) {
        return addAttribute(location, binding, Format::R32G32_SFLOAT, offset, "tex_coord");
    }

    /**
     * @brief Добавить атрибут цвета (vec4)
     */
    VertexLayout& addColor(uint32_t location, uint32_t binding, uint32_t offset) {
        return addAttribute(location, binding, Format::R32G32B32_SFLOAT, offset, "color");
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

    // ========================================================================
    // Conversion to Render Types
    // ========================================================================

    /**
     * @brief Конвертировать в Render::VertexInputStateDesc
     *
     * Используется при создании pipeline для настройки vertex input state.
     */
     VertexInputStateDesc toVertexInputStateDesc() const;

    // ========================================================================
    // Static Helpers - Predefined Layouts
    // ========================================================================

    /**
     * @brief Создать layout для простой вершины (только позиция)
     *
     * Ожидает вершину с struct { vec3 position; }
     * stride = sizeof(glm::vec3) = 12 байт
     */
    static VertexLayout createPositionOnly(uint32_t stride = 12);

    /**
     * @brief Создать layout для вершины с позицией и нормалью
     *
     * Ожидает вершину с struct { vec3 position; vec3 normal; }
     * stride = 24 байта
     */
    static VertexLayout createPositionNormal(uint32_t stride = 24);

    /**
     * @brief Создать layout для вершины с позицией, нормалью и UV
     *
     * Ожидает вершину с struct { vec3 position; vec3 normal; vec2 tex_coord; }
     * stride = 32 байта
     */
    static VertexLayout createPositionNormalTex(uint32_t stride = 32);

    /**
     * @brief Создать layout для вершины с позицией, нормалью, UV и цветом
     *
     * Ожидает вершину с struct { vec3 position; vec3 normal; vec2 tex_coord; vec4 color; }
     * stride = 48 байт
     */
    static VertexLayout createPositionNormalTexColor(uint32_t stride = 48);

    /**
     * @brief Создать пустой layout (для материалов без геометрии)
     */
    static VertexLayout createEmpty();

private:
    std::vector<Binding> m_bindings;
    std::vector<Attribute> m_attributes;
};

// ============================================================================
// Inline Implementation
// ============================================================================

inline VertexInputStateDesc VertexLayout::toVertexInputStateDesc() const {
    VertexInputStateDesc desc;

    // Convert bindings
    for (const auto& binding : m_bindings) {
        desc.bindings.push_back({
            .binding = binding.binding,
            .stride = binding.stride
        });
    }

    // Convert attributes
    for (const auto& attr : m_attributes) {
        desc.attributes.push_back({
            .location = attr.location,
            .binding = attr.binding,
            .format = attr.format,
            .offset = attr.offset
        });
    }

    return desc;
}

// Convenience static factories
inline VertexLayout VertexLayout::createPositionOnly(uint32_t stride) {
    VertexLayout layout;
    layout.setSingleBinding(stride);
    layout.addPosition(0, 0, 0);
    return layout;
}

inline VertexLayout VertexLayout::createPositionNormal(uint32_t stride) {
    VertexLayout layout;
    layout.setSingleBinding(stride);
    layout.addPosition(0, 0, 0);
    layout.addNormal(1, 0, 12);  // normal после position (12 байт)
    return layout;
}

inline VertexLayout VertexLayout::createPositionNormalTex(uint32_t stride) {
    VertexLayout layout;
    layout.setSingleBinding(stride);
    layout.addPosition(0, 0, 0);
    layout.addNormal(1, 0, 12);
    layout.addTexCoord(2, 0, 24);  // tex_coord после normal (12 + 12 = 24)
    return layout;
}

inline VertexLayout VertexLayout::createPositionNormalTexColor(uint32_t stride) {
    VertexLayout layout;
    layout.setSingleBinding(stride);
    layout.addPosition(0, 0, 0);
    layout.addNormal(1, 0, 12);
    layout.addTexCoord(2, 0, 24);
    layout.addColor(3, 0, 32);  // color после tex_coord (24 + 8 = 32)
    return layout;
}

inline VertexLayout VertexLayout::createEmpty() {
    return VertexLayout();
}

} // namespace Core
