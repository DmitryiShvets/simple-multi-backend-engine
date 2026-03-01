#pragma once

/**
 * @brief Uniform Factory Registry
 * 
 * Registers factories that convert material data → UniformSet.
 * 
 * Architecture:
 * - Each material type registers a factory function at startup
 * - RuntimeInitSystem uses the registry to create uniforms without knowing material types
 * 
 * To add a new material:
 * 1. Define material params struct with material_type_name
 * 2. Add factory registration in registerUniformFactories()
 * 3. Define vertex type with getLayout() method
 * 4. Create pipeline config with vertex_layout set
 */

#include "uniform_set.h"
#include "material_params.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Render {

/**
 * @brief Factory function type for converting material data to UniformSet
 * 
 * Takes material component data and returns UniformSet for GPU.
 * Does NOT create materials or buffers - only converts data → uniforms.
 */
using UniformFactoryFunc = std::function<UniformSet(const void*)>;

/**
 * @brief Registry of uniform factories
 * 
 * Maps material type names to their uniform factories.
 * Each material type registers its factory at startup.
 * 
 * Usage:
 *   UniformFactoryRegistry::instance().registerFactory<AdsMaterial>("ads_material",
 *       [](const void* data) {
 *           const auto& mat = *static_cast<const AdsMaterial*>(data);
 *           UniformSet uniforms;
 *           uniforms.set("color", UniformValue(mat.color));
 *           return uniforms;
 *       });
 */
class UniformFactoryRegistry {
public:
    static UniformFactoryRegistry& instance() {
        static UniformFactoryRegistry registry;
        return registry;
    }

    /**
     * @brief Register a factory for a material type
     */
    template <typename MaterialType>
    void registerFactory(const std::string& name, UniformFactoryFunc factory) {
        factories_[name] = std::move(factory);
    }

    /**
     * @brief Get factory for a material type
     */
    const UniformFactoryFunc* getFactory(const std::string& name) const {
        auto it = factories_.find(name);
        return it != factories_.end() ? &it->second : nullptr;
    }

    /**
     * @brief Check if a material type is registered
     */
    bool hasFactory(const std::string& name) const {
        return factories_.find(name) != factories_.end();
    }

private:
    UniformFactoryRegistry() = default;
    std::unordered_map<std::string, UniformFactoryFunc> factories_;
};

/**
 * @brief Register all uniform factories
 * 
 * Call this at application startup before creating RuntimeInitSystem
 */
inline void registerUniformFactories() {
    auto& registry = UniformFactoryRegistry::instance();

    // =========================================================================
    // Default Material - converts color → UniformSet
    // =========================================================================
    registry.registerFactory<Core::MaterialParams::DefaultMaterial>(
        Core::MaterialParams::DefaultMaterial::material_type_name,
        [](const void* data) {
            const auto& mat = *static_cast<const Core::MaterialParams::DefaultMaterial*>(data);
            UniformSet uniforms;
            uniforms.set("color", UniformValue(mat.color));
            return uniforms;
        }
    );

    // =========================================================================
    // ADS Material - converts color → UniformSet
    // =========================================================================
    registry.registerFactory<Core::MaterialParams::AdsMaterial>(
        Core::MaterialParams::AdsMaterial::material_type_name,
        [](const void* data) {
            const auto& mat = *static_cast<const Core::MaterialParams::AdsMaterial*>(data);
            UniformSet uniforms;
            uniforms.set("color", UniformValue(mat.color));
            return uniforms;
        }
    );

    // =========================================================================
    // PBR Material - placeholder for future implementation
    // =========================================================================
    // registry.registerFactory<Core::MaterialParams::PbrMaterialParams>(
    //     Core::MaterialParams::PbrMaterialParams::material_type_name,
    //     [](const void* data) {
    //         const auto& mat = *static_cast<const Core::MaterialParams::PbrMaterialParams*>(data);
    //         UniformSet uniforms;
    //         // Add PBR-specific uniforms (textures, factors, etc.)
    //         // uniforms.setTexture("albedo", ...);
    //         // uniforms.setFloat("metallic", mat.metallic_factor);
    //         // uniforms.setFloat("roughness", mat.roughness_factor);
    //         return uniforms;
    //     }
    // );
}

} // namespace Render
