#include "uniform_layout.h"
#include <cstring>
#include <stdexcept>

namespace Core {

// ============================================================================
// Variable Alignment
// ============================================================================

size_t UniformLayout::Variable::alignment() const {
    return UniformLayout::getStd140Alignment(type);
}

// ============================================================================
// Std140 Rules
// ============================================================================

size_t UniformLayout::getStd140Alignment(UniformValue::Type type) {
    // std140 layout rules:
    // - scalar (float, int, bool): 4 bytes
    // - vec2: 8 bytes
    // - vec3, vec4: 16 bytes
    // - mat2: 8 bytes (2 floats)
    // - mat3: 16 bytes (like vec4)
    // - mat4: 16 bytes (4 vec4s)
    
    switch (type) {
        // Scalars
        case UniformValue::Type::Float:
        case UniformValue::Type::Int:
        case UniformValue::Type::Uint:
        case UniformValue::Type::Bool:
            return 4;
        
        // Vec2
        case UniformValue::Type::Vec2:
        case UniformValue::Type::IVec2:
        case UniformValue::Type::UVec2:
            return 8;
        
        // Vec3, Vec4
        case UniformValue::Type::Vec3:
        case UniformValue::Type::Vec4:
        case UniformValue::Type::IVec3:
        case UniformValue::Type::IVec4:
        case UniformValue::Type::UVec3:
        case UniformValue::Type::UVec4:
            return 16;
        
        // Matrices (all aligned to 16 in std140)
        case UniformValue::Type::Mat2:
        case UniformValue::Type::Mat3:
        case UniformValue::Type::Mat4:
            return 16;
        
        default:
            return 4;  // Default alignment
    }
}

size_t UniformLayout::getStd140Size(UniformValue::Type type) {
    switch (type) {
        // Scalars
        case UniformValue::Type::Float:
        case UniformValue::Type::Int:
        case UniformValue::Type::Uint:
        case UniformValue::Type::Bool:
            return 4;
        
        // Vec2
        case UniformValue::Type::Vec2:
        case UniformValue::Type::IVec2:
        case UniformValue::Type::UVec2:
            return 8;
        
        // Vec3, Vec4
        case UniformValue::Type::Vec3:
        case UniformValue::Type::Vec4:
        case UniformValue::Type::IVec3:
        case UniformValue::Type::IVec4:
        case UniformValue::Type::UVec3:
        case UniformValue::Type::UVec4:
            return type == UniformValue::Type::Vec3 || 
                   type == UniformValue::Type::IVec3 || 
                   type == UniformValue::Type::UVec3 ? 12 : 16;
        
        // Matrices
        case UniformValue::Type::Mat2:
            return 32;  // 2 × vec4 (std140 pads each column)
        
        case UniformValue::Type::Mat3:
            return 48;  // 3 × vec4 (std140 pads each column)
        
        case UniformValue::Type::Mat4:
            return 64;  // 4 × vec4
        
        default:
            return 4;
    }
}

// ============================================================================
// Add Variable
// ============================================================================

UniformLayout& UniformLayout::addVariable(const std::string& name, 
                                           UniformValue::Type type,
                                           size_t manual_offset) {
    Variable var;
    var.name = name;
    var.type = type;
    var.size = getStd140Size(type);
    
    if (manual_offset > 0) {
        // Manual offset (if user knows what they're doing)
        var.offset = manual_offset;
    } else {
        // Auto-calculate by std140
        if (!m_variables.empty()) {
            const auto& last = m_variables.back();
            size_t next_offset = last.offset + last.size;
            var.offset = alignUp(next_offset, var.alignment());
        } else {
            var.offset = 0;
        }
    }
    
    m_variables.push_back(var);
    m_computed = false;
    
    return *this;
}

// ============================================================================
// Compute Layout
// ============================================================================

void UniformLayout::computeLayout() {
    if (m_variables.empty()) {
        m_total_size = 0;
        m_computed = true;
        return;
    }

    // Recalculate offsets if needed
    size_t current_offset = 0;

    for (auto& var : m_variables) {
        // Align by variable alignment
        current_offset = alignUp(current_offset, var.alignment());
        var.offset = current_offset;

        // Move forward
        current_offset += var.size;
    }

    // Align total block size to 16 bytes (std140)
    m_total_size = alignUp(current_offset, 16);
    m_computed = true;
}

// ============================================================================
// Get Variable
// ============================================================================

const UniformLayout::Variable* UniformLayout::getVariable(const std::string& name) const {
    for (const auto& var : m_variables) {
        if (var.name == name) {
            return &var;
        }
    }
    return nullptr;
}

// ============================================================================
// Pack
// ============================================================================

std::vector<uint8_t> UniformLayout::pack(const UniformSet& uniform_set) const {
    if (!m_computed) {
        throw std::runtime_error("UniformLayout::computeLayout() must be called before pack()");
    }
    
    std::vector<uint8_t> buffer(m_total_size, 0);  // Zero-initialize
    packTo(uniform_set, buffer.data());
    
    return buffer;
}

void UniformLayout::packTo(const UniformSet& uniform_set, uint8_t* buffer) const {
    if (!m_computed) {
        throw std::runtime_error("UniformLayout::computeLayout() must be called before packTo()");
    }
    
    // Zero-initialize buffer
    std::memset(buffer, 0, m_total_size);

    // Copy each variable to its offset
    for (const auto& var : m_variables) {
        const UniformValue* value = uniform_set.get(var.name);

        if (value) {
            // Type validation
            if (value->getType() != var.type) {
                // Warning: type mismatch, but we'll copy anyway
                // Could throw error in debug mode
            }

            // Copy data
            std::memcpy(buffer + var.offset, value->data(), value->size());
        }
        // If value is missing, leave as 0 (already initialized)
    }
}

// ============================================================================
// Create Std140
// ============================================================================

UniformLayout UniformLayout::createStd140(std::initializer_list<Variable> vars) {
    UniformLayout layout;
    
    for (const auto& var : vars) {
        layout.addVariable(var.name, var.type);
    }
    
    layout.computeLayout();
    
    return layout;
}

} // namespace Core
