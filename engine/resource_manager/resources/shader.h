#pragma once
#include "core/resource_types.h"
#include "core/rid.h"
#include "resource.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ssme {

class ShaderModule : public Resource {
public:
  using ParamsType = ShaderModuleDesc;
  static constexpr ResourceId ID = ResourceId::SHADER;
  static constexpr uint32_t COMPONENTS = 1; // 1 RID for ShaderModule
  // Dynamic DS Layout count depends on shader

  ShaderModule(std::string uuid, const VecRefRD &devices,
               const ShaderModuleDesc &desc);
  ~ShaderModule() override;
  // --- Resource Interface ---
  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  // --- Public API for reflection ---
  const std::map<std::string, Binding> &getBindings() const {
    return m_bindings;
  }

  std::shared_ptr<UniformLayout> getLayout(const std::string &block_name) const;

  RID getModuleId() const { return m_module_rid; }
  std::vector<RID> getDescriptorLayouts() const;
  std::vector<PushConstantRange> getPushConstants() const;
  std::vector<VertexInputRequirement> getVertexInputRequirements() const;
  UniformSet createUniformSet(const std::string &block_name) const;
  bool containsUniformBlock(const std::string &block_name);

private:
  ShaderModuleDesc m_desc;
  RID m_module_rid = RID::INVALID;
  std::map<uint32_t, RID> m_ds_layout_ids;

  // Reflection results. key is variable/binding name
  std::map<std::string, Binding> m_bindings; // already stored in DescriptorLayout
  std::map<std::string, std::shared_ptr<UniformLayout>> m_binding_layouts;
  std::map<std::string, PushConstantRange> m_push_constants;
  std::map<std::string, std::shared_ptr<UniformLayout>>
      m_push_constants_layouts;

  std::map<uint32_t, DescriptorLayout> m_ds_layouts;
  std::vector<VertexInputRequirement> m_vertex_requirements;

  uint32_t m_required_components =
      COMPONENTS; // must be equial COMPONENTS + m_ds_layouts.size()
  uint32_t m_descriptor_set_count = 0;
};

} // namespace ssme
