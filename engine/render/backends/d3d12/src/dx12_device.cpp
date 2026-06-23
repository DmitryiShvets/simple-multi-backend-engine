#include "dx12_device.h"
#include "com_exception.h"
#include "core/gpu_types.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>

namespace ssme::d3d12 {
Dx12Device::Dx12Device(ssme::Platform *platform) : m_platform(platform) {
    initialize();
}
Dx12Device::~Dx12Device() {}
void Dx12Device::initialize() {
  UINT dxgiFactoryFlags = 0;
// #ifndef NDEBUG
//   // Enable the debug layer (requires the Graphics Tools "optional feature").
//   // NOTE: Enabling the debug layer after device creation will invalidate the
//   // active device.
//   {
//     Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
//     if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
//       debugController->EnableDebugLayer();

//       // Enable additional debug layers.
//       dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
//     }
//   }
// #endif

  DX::ThrowIfFailed(
      CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
  if (m_use_warp_device) {
    Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
    DX::ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
    DX::ThrowIfFailed(D3D12CreateDevice(
        warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
  } else {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    getHardwareAdapter(m_factory.Get(), &hardwareAdapter, true);
    DX::ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(),
                                        D3D_FEATURE_LEVEL_11_0,
                                        IID_PPV_ARGS(&m_device)));
  }

  // Describe and create the command queue.
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  DX::ThrowIfFailed(
      m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_command_queue)));

}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void Dx12Device::getHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

    Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if(adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}
} // namespace ssme::d3d12
