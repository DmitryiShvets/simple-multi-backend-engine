#pragma once
#include "rid.h"

namespace Core::Ecs::Component {
    // RUNTIME ONLY
    struct VkRuntime {
        RID geometry_id;
        RID material_id;
        bool visible = true;
    };
    struct GlRuntime {
        RID geometry_id;
        RID material_id;
        bool visible = true;
    };
}
