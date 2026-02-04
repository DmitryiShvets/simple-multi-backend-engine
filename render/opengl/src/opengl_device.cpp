#include "opengl_device.h"

namespace Render::OpenGL {

OpenGLDevice::OpenGLDevice() {}
OpenGLDevice::~OpenGLDevice() {}

RID OpenGLDevice::createBuffer(const BufferDesc& desc) { return {}; }
RID OpenGLDevice::createTexture(const TextureDesc& desc) { return {}; }
RID OpenGLDevice::createSampler(const SamplerDesc& desc) { return {}; }
RID OpenGLDevice::createGraphicsPipeline(const GraphicsPipelineDesc& desc) { return {}; }
void OpenGLDevice::free(RID rid) {}

CommandList* OpenGLDevice::beginCommandList() { return nullptr; }
void OpenGLDevice::submitCommandLists(std::span<CommandList*> lists) {}

void OpenGLDevice::waitIdle() {}
void OpenGLDevice::tick() {}

} // namespace Render::OpenGL
