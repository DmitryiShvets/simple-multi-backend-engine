#pragma once
#include "core/render_types.h"
#include <d3d12.h>

namespace ssme::d3d12 {

D3D12_RESOURCE_STATES toD3d12State(ImageLayout layout);

}
