#include "dx12_texture.h"
#include "dx12_device.h"
#include <d3d12.h>

namespace ssme::d3d12 {

static bool hasFlag(ImageUsage usage, ImageUsage flag) {
  return (static_cast<uint32_t>(usage) & static_cast<uint32_t>(flag)) != 0;
}

Dx12Texture::Dx12Texture(Dx12Device &device,
                         Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                         DXGI_FORMAT format,
                         D3D12_CPU_DESCRIPTOR_HANDLE rtv_slot)
    : m_device(device), m_texture(resource), m_rtv_handle(rtv_slot),
      m_format(format) {}

Dx12Texture::Dx12Texture(Dx12Device &device, const TextureDesc &desc)
    : m_device(device), m_owns(true), m_width(desc.width),
      m_height(desc.height) {
  const bool is_depth = hasFlag(desc.usage, ImageUsage::DEPTH_STENCIL);
  m_is_depth = is_depth;
  m_format = toDxgiFormat(desc.format); // ресурсный формат

  DXGI_FORMAT view_format = m_format;
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  D3D12_CLEAR_VALUE clear{};

  if (is_depth) {
    flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    view_format = DXGI_FORMAT_D32_FLOAT;
    clear.Format = DXGI_FORMAT_D32_FLOAT;
    clear.DepthStencil.Depth = 1.0f;
  } else {
    flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (desc.format == Format::R8G8B8A8_SRGB)
      view_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // sRGB на уровне вью
    clear.Format = view_format;
  }

  D3D12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(
      m_format, desc.width, desc.height, 1, 1, 1, 0, flags);

  CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_DEFAULT);
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommittedResource(
      &heap_props, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_COMMON,
      &clear, IID_PPV_ARGS(&m_texture)));

  auto device = m_device.getHandle();
  if (is_depth) {
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NumDescriptors = 1;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DX::ThrowIfFailed(
        device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_dsv_heap)));
    m_dsv_handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
    dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
    dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    device->CreateDepthStencilView(m_texture.Get(), &dsv_desc, m_dsv_handle);
  } else {
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NumDescriptors = 1;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DX::ThrowIfFailed(
        device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_rtv_heap)));
    m_rtv_handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
    rtv_desc.Format = view_format;
    rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, m_rtv_handle);
  }
}
} // namespace ssme::d3d12
