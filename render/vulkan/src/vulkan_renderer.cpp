#include "vulkan_renderer.h"

namespace Render::Vulkan {

void VulkanRenderer::initialize(int width, int height) {}

void VulkanRenderer::destroy() {}

void VulkanRenderer::drawBundle(const std::string &shader, void *bundle) {}

void VulkanRenderer::frame(float delta_time) {}

void VulkanRenderer::resize(int width, int height) {}

void VulkanRenderer::setClearColor(float r, float g, float b, float a) {}

void VulkanRenderer::clear() {}

void VulkanRenderer::setViewPort(int x, int y, int width, int height) {}

} // namespace Render::Vulkan
