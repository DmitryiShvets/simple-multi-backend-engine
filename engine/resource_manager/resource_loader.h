#pragma  once

#include <string>
#include "core/resource_types.h"

namespace ssme {

class ResourceManager;

class IResourceLoader {
public:
    virtual ~IResourceLoader() = default;

    /**
     * @brief Returns the resource type this loader can load.
     */
    virtual ResourceId getResourceId() const = 0;

    /**
     * @brief Main loading method.
     * @param path File path (relative to res/)
     * @param rm Reference to resource manager (for nested dependencies)
     * @param out_params Pointer to Params structure (MaterialParams, TextureDesc, etc.)
     * @return true if file was successfully read and params are filled
     */
    virtual bool load(const std::string& path, ResourceManager& rm, void* out_params) = 0;
};

} // namespace ssme
