#pragma once
#include "drawing_policy.h"
#include <string>
#include <unordered_map>

namespace Render {

// DrawingPolicyRegistry - stores drawing policies for each material type
// Backend-agnostic: accessed by both Vulkan and OpenGL executors
class DrawingPolicyRegistry {
public:
  DrawingPolicyRegistry() = default;

  void init();
  // Register a policy for a material type
  void add(const std::string &material_type, DrawingPolicy policy);

  // Get policy by material type (returns nullptr if not found)
  const DrawingPolicy *get(const std::string &material_type) const;

  // Check if policy exists
  bool contains(const std::string &material_type) const;

private:
  std::unordered_map<std::string, DrawingPolicy> m_policies;
};

} // namespace Render
