#include "ads_pipeline.h"
#include "core/gpu_types.h"
#include "core/render_types.h"
#include "../pipeline_config_registry.h"
#include "core/vertex.h"
#include "core/vertex_layout.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace ssme {

namespace {

constexpr std::string mat_name = "ads";

// Vulkan uses 3-level descriptor set architecture:
// - Set 0: Per-frame uniforms (projectionViewMatrix) - created at runtime in
// VulkanRenderer
// - Set 1: Per-material uniforms (color) - created here
// - Set 2: Per-object uniforms (normal matrix) - created here
// - Push constants: Model matrix - set per-object
void initVulkanMaterial(PipelineConfigRegistry &registry) {
  PipelineConfig vk_default_mat = PipelineConfig::create(
      mat_name,
      // Vulkan configuration
      [](PipelineDesc &desc) {
        // Descriptor Set Layouts:
        // Set 0 (Per-frame) - GlobalUBO (proj matrix)
        // Set 1 (Per-material) - MaterialUBO (color)
        // Set 2 (Per-object) - ObjectUBO (normal matrix)
        desc.ds_layouts_desc.resize(3);
        desc.ds_layouts_desc[0].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(
                 ShaderStage::VERTEX)}}; // Set 0, Binding 0: GlobalUBO
        desc.ds_layouts_desc[1].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(
                 ShaderStage::VERTEX)}}; // Set 1, Binding 0: MaterialUBO
        desc.ds_layouts_desc[2].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(
                 ShaderStage::VERTEX)}}; // Set 2, Binding 0: ObjectUBO

        // Push constants for model matrix
        desc.pl_layout_desc.push_constant_ranges.push_back(
            {static_cast<uint32_t>(ShaderStage::VERTEX), 0, sizeof(glm::mat4)});
        desc.pl_desc.name = "ads";
        desc.pl_desc.shader_modules = {
            {"res/shaders/v_ads.glsl.spv", ShaderStage::VERTEX},
            {"res/shaders/f_default.glsl.spv", ShaderStage::FRAGMENT}};
        desc.pl_desc.vertex_layout = VertexN::getLayout();
        desc.pl_desc.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
      });

  // Uniform layout for material uniforms (Set 1)
  // Must match shader layout!
  vk_default_mat.uniform_layout = UniformLayout::createStd140({
      UniformLayout::Variable{.name = "color",
                              .offset = 0,
                              .size = 12, // vec3 = 12 bytes
                              .type = UniformValue::Type::Vec3}
      // std140 automatically adds 4 bytes padding to 16 bytes
  });
  vk_default_mat.uniform_layout.computeLayout();

  // Uniform layout for object uniforms (Set 2)
  // Must match shader layout!
  vk_default_mat.object_uniform_layout = UniformLayout::createStd140({
      UniformLayout::Variable{.name = "model_mat1",
                              .offset = 0,
                              .size = sizeof(glm::mat4), // mat4 = 64 bytes
                              .type = UniformValue::Type::Mat4},
      UniformLayout::Variable{.name = "normal_mat",
                              .offset = sizeof(glm::mat4),
                              .size = 48, // mat3 in std140 = 48 bytes NOT 36
                              .type = UniformValue::Type::Mat3}
      // std140: mat3 is padded to 48 bytes (3 * vec4)
  });
  vk_default_mat.object_uniform_layout.computeLayout();

  registry.add(std::move(vk_default_mat));
}

void initOpenglMaterial(PipelineConfigRegistry &registry) {
  PipelineConfig gl_default_mat = PipelineConfig::create(
      mat_name,
      // OpenGL configuration
      [](PipelineDesc &desc) {
        // OpenGL uses global binding space (no per-set isolation like Vulkan)
        // Set 0 → Global Binding 0, Set 1 → Global Binding 1
        desc.ds_layouts_desc.resize(3);
        desc.ds_layouts_desc[0].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(
                 ShaderStage::VERTEX)}}; // Set 0, Binding 0: GlobalUBO
        desc.ds_layouts_desc[1].bindings = {
            {1, DescriptorType::UNIFORM_BUFFER, // ← OpenGL: binding=1 (not 0!)
             static_cast<uint32_t>(ShaderStage::VERTEX)}};
        desc.ds_layouts_desc[2].bindings = {
            {2, DescriptorType::UNIFORM_BUFFER, // ← OpenGL: binding=2 (not 0!)
             static_cast<uint32_t>(
                 ShaderStage::VERTEX)}}; // Set 2, Binding 2: ObjectUBO
                                         // (global binding space)

        desc.pl_desc.name = "ads";
        desc.pl_desc.shader_modules = {
            {"res/shaders/v_ads.glsl", ShaderStage::VERTEX},
            {"res/shaders/f_default.glsl", ShaderStage::FRAGMENT}};
        desc.pl_desc.vertex_layout = VertexN::getLayout();
        desc.pl_desc.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
      });

  // Uniform layout for material uniforms (Set 1)
  // Must match shader layout!
  gl_default_mat.uniform_layout = UniformLayout::createStd140({
      UniformLayout::Variable{.name = "color",
                              .offset = 0,
                              .size = 12, // vec3 = 12 bytes
                              .type = UniformValue::Type::Vec3}
      // std140 automatically adds 4 bytes padding to 16 bytes
  });
  gl_default_mat.uniform_layout.computeLayout();

  // Uniform layout for object uniforms (Set 2)
  // Must match shader layout!
  gl_default_mat.object_uniform_layout = UniformLayout::createStd140({
      UniformLayout::Variable{.name = "model_mat1",
                              .offset = 0,
                              .size = sizeof(glm::mat4), // mat4 = 64 bytes
                              .type = UniformValue::Type::Mat4},
      UniformLayout::Variable{.name = "normal_mat",
                              .offset = sizeof(glm::mat4),
                              .size = 48, // mat3 in std140 = 48 bytes NOT 36
                              .type = UniformValue::Type::Mat3}
      // std140: mat3 is padded to 48 bytes (3 * vec4)
  });
  gl_default_mat.object_uniform_layout.computeLayout();

  registry.add(std::move(gl_default_mat));
}

} // namespace

void AdsPipeline::addConfigToRegistry(PipelineConfigRegistry &registry,
                                      GpuBackend type) {
  if (type == GpuBackend::OpenGL)
    initOpenglMaterial(registry);
  if (type == GpuBackend::Vulkan)
    initVulkanMaterial(registry);
}


} // namespace ssme
