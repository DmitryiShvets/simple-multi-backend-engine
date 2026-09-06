#pragma once

#include "core/uniform_set.h"
#include "resource.h"
#include "resource_handle.h"
#include "resources/shader.h"
#include "resources/uniform_block.h"
#include "texture.h"
#include <map>
#include <string>


// Material does not include pipeline.. these are different entities with different
// goals. Material — is Data (parameters + textures). Pipeline — is State
// (shader + GPU settings + vertex format). The same material (e.g.
// "Red plastic") can be rendered by different pipelines (one for regular
// meshes, another for instancing, third for shadows).

/**
 * @brief Material resource
 *
 * Goals and requirements:
 * - Universality (Type Erasure): No AdsMaterial, WaterMaterial in engine code.
 *   Class works with any data set via UniformLayout.
 * - Binding: Material knows which Pipeline (shader + GPU settings) it needs.
 * - Data isolation: Parameters (color, gloss, transparency) stored in raw binary
 *   form, packed by std140 standard, so GPU can read them without intermediate conversions.
 * - Resource management: Material owns texture handles (ResourceHandle<Texture>),
 *   so textures won't be unloaded while material exists.
 * - Single RID (Uniform Buffer): Each material instance creates its own small
 *   Uniform Buffer (or share in large pool) for storing unique settings.
 */

namespace ssme {

/**
 * @brief Material params for creating a Material instance.
 */
struct MaterialParams {
  ResourceHandle<ShaderModule> vert_shader;
  ResourceHandle<ShaderModule> frag_shader;
  // Named uniform blocks (e.g., "MaterialBlock", "LightingBlock")
  std::map<std::string, UniformSet> blocks_data;
  std::map<std::string, ResourceHandle<Texture>> textures;
  std::map<std::string, ResourceHandle<UniformBuffer>> uniforms;
};

class Material : public Resource {
public:
  using ParamsType = MaterialParams;
  static constexpr ResourceId ID = ResourceId::MATERIAL;
  static constexpr uint32_t COMPONENTS = 1; // Creates DS

  Material(std::string uuid, const VecRefRD &devices,
           const MaterialParams &desc);
  ~Material() override;

  // Resource interface implementation
  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  // Public API for Renderer
  const VecRID & getDescriptors() const { return m_ubo_ds;}
  std::map<std::string, RID> getTextures() const;
  RID getTextureDescriptor() const { return m_tex_ds; }

  ResourceHandle<ShaderModule> getVertShader() const { return m_vert_shader; }
  ResourceHandle<ShaderModule> getFragShader() const { return m_frag_shader; }
  /**
   * @brief Get UniformSet for some uniform block
   * @param block_name name of uniform block
   * @return UniformSet&
   */
  UniformSet &block(const std::string &block_name);

private:
  ResourceHandle<ShaderModule> m_vert_shader;
  ResourceHandle<ShaderModule> m_frag_shader;
  std::map<std::string, UniformSet> m_blocks_data;
  std::map<std::string, ResourceHandle<Texture>> m_textures;
  std::map<std::string, ResourceHandle<UniformBuffer>> m_uniforms;

  VecRID m_ubo_ds;
  VecRID m_ubo_ds_layouts;

  RID m_tex_ds_layout = RID::INVALID;
  RID m_tex_ds = RID::INVALID;

  uint32_t m_requred_components = COMPONENTS;

};

} // namespace ssme
