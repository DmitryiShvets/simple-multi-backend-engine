#include "drawing_policy_registry.h"
#include "../drawing_policies/default_drawing_policy.h"

namespace Render {

 void DrawingPolicyRegistry::init() {
     add("default", DefaultDrawingPolicy::create());
 }

void DrawingPolicyRegistry::add(const std::string &material_type,
                                            DrawingPolicy policy) {
    m_policies[material_type] = policy;
}

const DrawingPolicy *DrawingPolicyRegistry::get(const std::string &material_type) const {
    auto it = m_policies.find(material_type);
    if (it == m_policies.end()) {
        return nullptr;
    }
    return &it->second;
}

bool DrawingPolicyRegistry::contains(const std::string &material_type) const {
    return m_policies.find(material_type) != m_policies.end();
}

} // namespace Render
