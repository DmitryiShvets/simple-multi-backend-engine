#include "dx12_texture.h"
#include "com_exception.h"
#include "core/resource_types.h"
#include "d3dx12.h"
#include "dx12_command_list.h"
#include "dx12_device.h"
#include "dx12_helpers.h"
#include "utils/image_loader.h"
#include <d3d12.h>

namespace ssme::d3d12 {

static bool hasFlag(ImageUsage usage, ImageUsage flag) {
  return (static_cast<uint32_t>(usage) & static_cast<uint32_t>(flag)) != 0;
}


Dx12Texture::Dx12Texture(Dx12Device& device, const std::string& filepath)
    : m_device(device), m_owns(true) {
  int w, h, channels;
  auto* pixels = loadImage(filepath.c_str(), &w, &h, &channels);
  if (!pixels) throw std::runtime_error("Failed to load texture image!");
  m_width = w; m_height = h;
  m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

  m_pixels.assign(pixels, pixels + size_t(w) * h * 4); // stbi force 4ch
  freeImage(pixels);

  // Текстура в COPY_DEST сразу — как в MS-сэмплах, без reliance на promotion
  D3D12_RESOURCE_DESC tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(
      m_format, w, h, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE);
  CD3DX12_HEAP_PROPERTIES default_heap(D3D12_HEAP_TYPE_DEFAULT);
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommittedResource(
      &default_heap, D3D12_HEAP_FLAG_NONE, &tex_desc,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_texture)));

  // Staging — член; будет жить до upload()+wait
  UINT64 size = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);
  CD3DX12_HEAP_PROPERTIES upload_heap(D3D12_HEAP_TYPE_UPLOAD);
  auto buf = CD3DX12_RESOURCE_DESC::Buffer(size);
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommittedResource(
      &upload_heap, D3D12_HEAP_FLAG_NONE, &buf,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_upload_heap)));

  // CPU-only SRV источник (дескриптор из него копируется в set'ы)
  D3D12_DESCRIPTOR_HEAP_DESC hd{};
  hd.NumDescriptors = 1;
  hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // не SHADER_VISIBLE
  DX::ThrowIfFailed(
      m_device.getHandle()->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&m_srv_heap)));
  m_srv_handle = m_srv_heap->GetCPUDescriptorHandleForHeapStart();
  D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
  srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  // srv.Format = m_format;
  srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv.Texture2D.MipLevels = 1;
  m_device.getHandle()->CreateShaderResourceView(
      m_texture.Get(), &srv, m_srv_handle);

  upload();
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
    m_format = DXGI_FORMAT_R32_TYPELESS;
    view_format = DXGI_FORMAT_D32_FLOAT;
    clear.Format = DXGI_FORMAT_D32_FLOAT;
    clear.DepthStencil.Depth = 1.0f;
  } else {
    flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (desc.format == Format::R8G8B8A8_SRGB)
      view_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // sRGB на уровне вью
    clear.Format = view_format;
    // TODO: delete hardcode. make it data driven
    clear.Color[0] = 0.1f;
    clear.Color[1] = 0.1f;
    clear.Color[2] = 0.1f;
    clear.Color[3] = 1.0f;
  }

  D3D12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(
      m_format, desc.width, desc.height, 1, 1, 1, 0, flags);

  CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_DEFAULT);
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommittedResource(
      &heap_props, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_COMMON,
      &clear, IID_PPV_ARGS(&m_texture)));

  auto dx_device = m_device.getHandle();
  if (is_depth) {
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NumDescriptors = 1;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DX::ThrowIfFailed(
        dx_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_dsv_heap)));
    m_dsv_handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
    dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
    dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dx_device->CreateDepthStencilView(m_texture.Get(), &dsv_desc, m_dsv_handle);
  } else {
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NumDescriptors = 1;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DX::ThrowIfFailed(
        dx_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_rtv_heap)));
    m_rtv_handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
    rtv_desc.Format = view_format;
    rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    dx_device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, m_rtv_handle);
  }
}

void Dx12Texture::upload() {
  auto cmd = m_device.beginSingleTimeCommands();
  D3D12_SUBRESOURCE_DATA data{};
  data.pData      = m_pixels.data();
  data.RowPitch   = m_width * 4;
  data.SlicePitch = data.RowPitch * m_height;
  UpdateSubresources(cmd.Get(), m_texture.Get(), m_upload_heap.Get(),
                     0, 0, 1, &data);
  D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  cmd->ResourceBarrier(1, &barrier);
  m_device.endSingleTimeCommands(cmd);
  m_upload_heap.Reset();  // staging can be freid
  m_pixels.clear();
}

} // namespace ssme::d3d12
