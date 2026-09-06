#include "dx12_device.h"
#include "com_exception.h"
#include "core/gpu_types.h"
#include "utils/logger.h"
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <vector>
#include <windows.h>
#include <wrl.h>

namespace {
#ifndef NDEBUG
void CALLBACK Dx12DebugCallback(D3D12_MESSAGE_CATEGORY Category,
                                D3D12_MESSAGE_SEVERITY Severity,
                                D3D12_MESSAGE_ID ID, LPCSTR pDescription,
                                void *pContext) {
  if (Severity == D3D12_MESSAGE_SEVERITY_ERROR ||
      Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) {
    ssme::Logger::validation_log("[DX12 ERROR]", pDescription);
  } else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING) {
    ssme::Logger::validation_log("[DX12 WARNING]", pDescription);
  }
}
#endif
} // namespace

namespace ssme::d3d12 {
Dx12Device::Dx12Device(ssme::Platform *platform) : m_platform(platform) {
  initialize();
}
Dx12Device::~Dx12Device() {
    if (m_single_time_fence_event) CloseHandle(m_single_time_fence_event);
}

void Dx12Device::flushD3D12Messages() {
  ID3D12InfoQueue *q = m_info_queue.Get();
  if (!q)
    return;
  UINT64 msgCount = q->GetNumStoredMessages();
  for (UINT64 i = 0; i < msgCount; ++i) {
    SIZE_T msgLen = 0;
    q->GetMessage(i, nullptr, &msgLen);
    std::vector<BYTE> msgBuf(msgLen);
    D3D12_MESSAGE *msg = reinterpret_cast<D3D12_MESSAGE *>(msgBuf.data());
    q->GetMessage(i, msg, &msgLen);
    // Выводим в ваш лог
    OutputDebugStringA(msg->pDescription);
    OutputDebugStringA("\n");
  }
  q->ClearStoredMessages();
}

void Dx12Device::initialize() {
  UINT dxgiFactoryFlags = 0;

  if (g_enable_validation_layers) {
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the
    // active device.
    {
      Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
      if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();

        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
      }
    }
  }

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
  if (g_enable_validation_layers) {
    if (SUCCEEDED(m_device.As(&m_info_queue))) {
      // Configure an immediate stop (Break) in the debugger upon critical
      // errors
      m_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
      m_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
      m_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);

      // Filtering spam (hiding purely informational messages at the INFO level)
      D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO,
                                             D3D12_MESSAGE_SEVERITY_ERROR,
                                             D3D12_MESSAGE_SEVERITY_WARNING};

      D3D12_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumSeverities = _countof(severities);
      filter.DenyList.pSeverityList = severities;

      m_info_queue->PushStorageFilter(&filter);
    }

    // Register a callback function to redirect logs to the standard console.
    // To do this, request the updated ID3D12InfoQueue1 interface.
    Microsoft::WRL::ComPtr<ID3D12InfoQueue1> infoQueue1;
    if (SUCCEEDED(m_device.As(&infoQueue1))) {
      DWORD callbackCookie = 0;
      infoQueue1->RegisterMessageCallback(Dx12DebugCallback,
                                          D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                                          nullptr, &callbackCookie);
    }
  }
  // Describe and create the command queue.
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  DX::ThrowIfFailed(
      m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_command_queue)));
}

// Helper function for acquiring the first available hardware adapter that
// supports Direct3D 12. If no such adapter can be found, *ppAdapter will be set
// to nullptr.
_Use_decl_annotations_ void
Dx12Device::getHardwareAdapter(IDXGIFactory1 *pFactory,
                               IDXGIAdapter1 **ppAdapter,
                               bool requestHighPerformanceAdapter) {
  *ppAdapter = nullptr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

  Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
  if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
    for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
             adapterIndex,
             requestHighPerformanceAdapter == true
                 ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                 : DXGI_GPU_PREFERENCE_UNSPECIFIED,
             IID_PPV_ARGS(&adapter)));
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create
      // the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                      _uuidof(ID3D12Device), nullptr))) {
        break;
      }
    }
  }

  if (adapter.Get() == nullptr) {
    for (UINT adapterIndex = 0;
         SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter));
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create
      // the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                      _uuidof(ID3D12Device), nullptr))) {
        break;
      }
    }
  }

  *ppAdapter = adapter.Detach();
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>
Dx12Device::beginSingleTimeCommands() {
  if (!m_single_time_allocator) {
    DX::ThrowIfFailed(m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_single_time_allocator)));
  }
  m_single_time_allocator->Reset();  // безопасно: прошлый аплоад уже дождались
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list;
  DX::ThrowIfFailed(m_device->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_single_time_allocator.Get(),
      nullptr, IID_PPV_ARGS(&cmd_list)));
  return cmd_list;
}

void Dx12Device::endSingleTimeCommands(
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd_list) {
  DX::ThrowIfFailed(cmd_list->Close());
  ID3D12CommandList* lists[] = {cmd_list.Get()};
  m_command_queue->ExecuteCommandLists(1, lists);

  if (!m_single_time_fence) {
    DX::ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                            IID_PPV_ARGS(&m_single_time_fence)));
  }
  UINT64 value = ++m_single_time_fence_value;
  DX::ThrowIfFailed(m_command_queue->Signal(m_single_time_fence.Get(), value));
  if (!m_single_time_fence_event) {
    m_single_time_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  }
  m_single_time_fence->SetEventOnCompletion(value, m_single_time_fence_event);
  WaitForSingleObject(m_single_time_fence_event, INFINITE);
}

} // namespace ssme::d3d12
