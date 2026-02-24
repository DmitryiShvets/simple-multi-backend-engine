#pragma once

#include <glad/gl.h>
#include <vector>
#include <cstdint>

namespace Render::OpenGL {

// Descriptor binding - связка UBO/текстуры с binding point
struct DescriptorBinding {
    GLuint handle;        // Handle ресурса (UBO, texture и т.д.)
    uint32_t binding;     // Binding point (должен совпадать с шейдером!)
};

// OpenGL DescriptorSet - эмуляция Vulkan DescriptorSet
// Хранит коллекцию биндингов и биндит их на OpenGL context
class OpenGLDescriptorSet {
public:
    OpenGLDescriptorSet() = default;
    ~OpenGLDescriptorSet() = default;

    // Добавить binding
    void addBinding(uint32_t binding, GLuint handle);

    // Забиндить все ресурсы в OpenGL context
    void bind() const;

    // Получить количество биндингов
    size_t getBindingCount() const { return m_bindings.size(); }

    // Получить binding по индексу
    const DescriptorBinding& getBinding(size_t index) const { return m_bindings[index]; }

private:
    std::vector<DescriptorBinding> m_bindings;
};

// Descriptor Set Layout для OpenGL
// Хранит метаданные о структуре DescriptorSet (для валидации)
class OpenGLDescriptorSetLayout {
public:
    struct BindingDesc {
        uint32_t binding;
        uint32_t type;  // DescriptorType
        uint32_t stages; // ShaderStageFlags
        uint32_t count;
    };

    OpenGLDescriptorSetLayout() = default;
    
    // Добавить описание binding'а
    void addBinding(uint32_t binding, uint32_t type, uint32_t stages, uint32_t count = 1);

    // Получить количество binding'ов
    size_t getBindingCount() const { return m_bindings.size(); }

    // Получить binding по индексу
    const BindingDesc& getBinding(size_t index) const { return m_bindings[index]; }

private:
    std::vector<BindingDesc> m_bindings;
};

} // namespace Render::OpenGL
