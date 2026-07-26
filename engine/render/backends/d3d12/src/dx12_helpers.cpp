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

DXGI_FORMAT toDxgiFormat(Format format) {
  switch (format) {
  case Format::R32G32B32A32_SFLOAT:
    return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case Format::R32G32B32_SFLOAT:
    return DXGI_FORMAT_R32G32B32_FLOAT;
  case Format::R32G32_SFLOAT:
    return DXGI_FORMAT_R32G32_FLOAT;
  case Format::R32_SFLOAT:
    return DXGI_FORMAT_R32_FLOAT;
  case Format::R8G8B8A8_UNORM:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

} // namespace ssme::d3d12
