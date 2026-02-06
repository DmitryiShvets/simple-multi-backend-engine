#pragma once
#include <cstdint>

namespace Render {


    enum class ResourceType {
        UNDEFINED,
        SWAP_CHAIN,
        TEXTURE,
        BUFFER,
        PIPELINE,
    };

    enum class ResourceState {
        UNDEFINED,
        TRANSFER_DST,
        PRESENT_SRC
    };

    struct SwapChainDesc {
        void* native_window_handle;
        uint32_t width;
        uint32_t height;
        bool vsync;
    };
}
