#pragma once
#include <wrl/client.h>
// Forward declaration
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandAllocator;
struct ID3D12CommandQueue;
struct ID3D12PipelineState;
struct IDXGIFactory1;
struct IDXGIFactory4;
struct IDXGIAdapter1;

namespace ssme {
class Platform;
}
namespace ssme::d3d12 {

class Dx12Device {
public:
  /**
   * @brief Constructs the device, initializing handles to null.
   */
  Dx12Device(ssme::Platform *platform);

  /**
   * @brief Destroys all created Dx12 objects in the correct order.
   */
  ~Dx12Device();

  /**
   * @brief Main initialization method that triggers the entire Dx12 setup
   * chain.
   * @param creator A strategy object for creating the platform-specific
   * Surface.
   */
  void initialize();

  // --- Getters for foundational DirectX objects ---
  Microsoft::WRL::ComPtr<ID3D12Device> const &getHandle() const {
    return m_device;
  }
  Microsoft::WRL::ComPtr<IDXGIFactory4> const &getFactory() const & {
    return m_factory;
  }
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> const &getCommandQueue() const & {
    return m_command_queue;
  }

private:
  void getHardwareAdapter(_In_ IDXGIFactory1 *pFactory,
                          _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter,
                          bool requestHighPerformanceAdapter = false);

  ssme::Platform *m_platform;
  // --- DirectX Objects ---
  Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory = nullptr;
  Microsoft::WRL::ComPtr<ID3D12Device> m_device = nullptr;
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue = nullptr;
  bool m_use_warp_device = false;

public:

};
} // namespace ssme::d3d12
