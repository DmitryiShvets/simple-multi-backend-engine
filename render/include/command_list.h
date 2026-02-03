#pragma once
#include "render_types.h"
namespace Render {
class CommandList {
    public:
    virtual ~CommandList() = default;

    virtual void begin() = 0;
    virtual void end() = 0;

    // Указываем, какой ресурс (по его RID) очистить
    virtual void clearRenderTarget(RID renderTarget, const float color[4]) = 0;

    // Указываем, у какого ресурса сменить состояние
    virtual void resourceBarrier(RID resource, ResourceState before, ResourceState after) = 0;
};
} // namespace Render
