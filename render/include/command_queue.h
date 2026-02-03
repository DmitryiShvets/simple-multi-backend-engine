#pragma once
#include "command_list.h"

namespace Render {
class CommandQueue {
    public:
        virtual ~CommandQueue() = default;
        virtual void executeCommandLists(CommandList* const* lists, uint32_t count) = 0;
};
} // namespace Render
