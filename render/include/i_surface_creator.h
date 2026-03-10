#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
/**
 * @class IVulkanSurfaceCreator
 * @brief An interface for creating a Vulkan rendering surface (VkSurfaceKHR)
 *        and providing required instance extensions.
 *
 * This class defines an abstract interface for platform-specific operations
 * required by a rendering device. It decouples the renderer from the windowing
 * system (like GLFW, SDL, etc.) by providing a contract for:
 * 1. Querying necessary Vulkan instance extensions for surface creation.
 * 2. Creating the actual VkSurfaceKHR.
 */
class IVulkanSurfaceCreator {
public:
    virtual ~IVulkanSurfaceCreator() = default;

    /**
     * @brief Gets the list of required Vulkan instance extensions.
     *
     * The Vulkan instance needs to be created with extensions that support
     * surface integration for the specific platform. This method returns
     * a list of these extension names.
     *
     * @return A vector of C-strings, where each string is a required
     *         Vulkan instance extension name.
     */
    virtual std::vector<const char*> getRequiredInstanceExtensions() const = 0;

    /**
     * @brief Creates a rendering surface.
     *
     * This method creates a platform-specific VkSurfaceKHR using the provided
     * Vulkan instance.
     *
     * @param instance The Vulkan instance to create the surface with.
     * @param surface A pointer to a VkSurfaceKHR handle where the created
     *                surface will be stored.
     * @return VkResult indicating the success or failure of the operation.
     */
    virtual VkSurfaceKHR createWindowSurface(vk::Instance instance) const = 0;
};
