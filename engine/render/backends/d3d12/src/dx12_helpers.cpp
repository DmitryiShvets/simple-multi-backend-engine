#include "dx12_helpers.h"

namespace ssme::d3d12 {

D3D12_RESOURCE_STATES toD3d12State(ImageLayout layout) {
  switch (layout) {
  case ImageLayout::UNDEFINED:
    return D3D12_RESOURCE_STATE_COMMON;
  case ImageLayout::COLOR_ATTACHMENT:
    return D3D12_RESOURCE_STATE_RENDER_TARGET;
  case ImageLayout::PRESENT_SRC:
    return D3D12_RESOURCE_STATE_PRESENT;
  case ImageLayout::TRANSFER_DST:
    return D3D12_RESOURCE_STATE_COPY_DEST;
  case ImageLayout::SHADER_READ_ONLY:
    return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  default:
    return D3D12_RESOURCE_STATE_COMMON;
  }
}

} // namespace ssme::d3d12
