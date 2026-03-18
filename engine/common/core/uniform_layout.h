#pragma once

#include "uniform_set.h"
#include "uniform_value.h"
#include <vector>
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

namespace ssme {

/**
 * @brief UniformLayout — описывает layout uniform блока для упаковки данных
 *
 * Содержит информацию об оффсетах, размерах и выравнивании каждой переменной.
 * Используется для упаковки UniformSet в binary buffer для GPU.
 *
 * Поддерживает std140 layout (требуется для Vulkan/OpenGL uniform buffers).
 */
class UniformLayout {
public:
    /**
     * @brief Описание одной переменной в layout
     */
    struct Variable {
        std::string name;           ///< Имя переменной (должно совпадать с шейдером!)
        size_t offset;              ///< Оффсет в байтах от начала buffer
        size_t size;                ///< Размер в байтах
        UniformValue::Type type;    ///< Тип для валидации

        /**
         * @brief Выравнивание по std140
         */
        size_t alignment() const;
    };

    /**
     * @brief Добавить переменную в layout
     * @param name Имя переменной
     * @param type Тип переменной
     * @param manual_offset Ручной оффсет (0 для авто-расчёта по std140)
     */
    UniformLayout& addVariable(const std::string& name,
                                UniformValue::Type type,
                                size_t manual_offset = 0);

    /**
     * @brief Получить переменную по имени
     */
    const Variable* getVariable(const std::string& name) const;

    /**
     * @brief Получить все переменные
     */
    const std::vector<Variable>& getVariables() const { return m_variables; }

    /**
     * @brief Получить общий размер buffer с учётом выравнивания
     */
    size_t getTotalSize() const { return m_total_size; }

    /**
     * @brief Вычислить размер и оффсеты по std140
     *
     * Вызывается после добавления всех переменных.
     */
    void computeLayout();

    /**
     * @brief Упаковать UniformSet в binary buffer
     *
     * @param uniform_set Данные для упаковки
     * @return Binary buffer размером getTotalSize()
     *
     * @note Переменные копируются по оффсетам из layout.
     *       Переменные без значения в uniform_set заполняются нулями.
     */
    std::vector<uint8_t> pack(const UniformSet& uniform_set) const;

    /**
     * @brief Упаковать в существующий buffer (без аллокаций)
     *
     * @param uniform_set Данные для упаковки
     * @param buffer Буфер для записи (должен быть >= getTotalSize())
     */
    void packTo(const UniformSet& uniform_set, uint8_t* buffer) const;

    /**
     * @brief Создать layout для std140 из списка переменных
     *
     * Convenience метод для быстрого создания layout.
     */
    static UniformLayout createStd140(std::initializer_list<Variable> vars);

private:
    /**
     * @brief Получить выравнивание для типа по std140
     */
    static size_t getStd140Alignment(UniformValue::Type type);

    /**
     * @brief Получить размер для типа по std140
     */
    static size_t getStd140Size(UniformValue::Type type);

    /**
     * @brief Выровнять значение вверх по alignment
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
