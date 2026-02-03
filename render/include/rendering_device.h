#pragma once

#include "command_list.h"
#include "render_types.h"

namespace Render {

// This is the main public interface for the Rendering Hardware Interface (RHI).
// The application will use this to create resources and perform rendering
// tasks.
class RenderingDevice {
public:
  virtual ~RenderingDevice() = default;

  // --- Resource Management ---

  // Creates a swap chain and returns its RID.
  virtual RID createSwapChain(const SwapChainDesc &desc) = 0;

  // Creates a buffer and returns its RID.
  // virtual RID createBuffer(const BufferDesc& desc) = 0;

  // Frees any resource by its RID.
  virtual void free(RID rid) = 0;

  // --- Frame Execution ---

  // A method that can be called once per frame for background tasks,
  // like releasing old resources.
  virtual void tick() = 0;

  // In the future, methods for starting/ending a frame and getting
  // command lists will be here.
  virtual CommandList *beginCommandList(RID swapChain) = 0;
  virtual void submitCommandList(
      RID swapChain,
      CommandList *list) = 0; // Упрощенная версия для одного списка

  // Методы для управления циклом кадра
  virtual RID acquireNextFrame(RID swapChain) = 0;
  virtual void present(RID swapChain) = 0;
  virtual void waitIdle() = 0;
};

} // namespace Render
