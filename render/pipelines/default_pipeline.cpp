#include "default_pipeline.h"
#include "pipeline_config_registry.h"
#include "vertex.h"
#include "vertex_layout.h"
#include <string>
#include <glm/gtc/matrix_transform.hpp>

namespace Render {

constexpr std::string mat_name = "default";

// Vulkan uses 2-level descriptor set architecture:
// - Set 0: Per-frame uniforms (projectionViewMatrix) - created at runtime in VulkanRenderer
// - Set 1: Per-material uniforms (color) - created here
// - Push constants: Model matrix - set per-object
void initVulkanMaterial(PipelineConfigRegistry &registry) {
  PipelineConfig vk_default_mat = PipelineConfig::create(
      mat_name,
      // Vulkan configuration
      [](PipelineDesc &desc) {
        // Descriptor Set Layouts:
        // Set 0 (Per-frame) - GlobalUBO (projectionViewMatrix)
        // Set 1 (Per-material) - MaterialUBO (color)
        desc.ds_layouts_desc.resize(2);
        desc.ds_layouts_desc[0].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(ShaderStage::VERTEX)}
        }; // Set 0, Binding 0: GlobalUBO
        desc.ds_layouts_desc[1].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(ShaderStage::VERTEX)}
        }; // Set 1, Binding 0: MaterialUBO (Vulkan: binding is per-set)

        // Push constants for model matrix
        desc.pl_layout_desc.push_constant_ranges.push_back({
            static_cast<uint32_t>(ShaderStage::VERTEX), 0, sizeof(glm::mat4)
        });
        desc.pl_desc.name = "default";
        desc.pl_desc.shader_modules = {
            {"res/shaders/v_default.glsl.spv", ShaderStage::VERTEX},
            {"res/shaders/f_default.glsl.spv", ShaderStage::FRAGMENT}};
        desc.pl_desc.vertex_layout = Vertex::getLayout();
        desc.pl_desc.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
      });

  // Uniform layout for material uniforms (Set 1)
  // Must match shader layout!
  vk_default_mat.uniform_layout = UniformLayout::createStd140({
      UniformLayout::Variable{
          .name = "color",
          .offset = 0,
          .size = 12,  // vec3 = 12 bytes
          .type = Core::UniformValue::Type::Vec3
      }
      // std140 automatically adds 4 bytes padding to 16 bytes
  });
  vk_default_mat.uniform_layout.computeLayout();

  registry.add(std::move(vk_default_mat));
}

void initOpenglMaterial(PipelineConfigRegistry &registry) {
  PipelineConfig gl_default_mat = PipelineConfig::create(
      mat_name,
      // OpenGL configuration
      [](PipelineDesc &desc) {
        // OpenGL uses global binding space (no per-set isolation like Vulkan)
        // Set 0 → Global Binding 0, Set 1 → Global Binding 1
        desc.ds_layouts_desc.resize(2);
        desc.ds_layouts_desc[0].bindings = {
            {0, DescriptorType::UNIFORM_BUFFER,
             static_cast<uint32_t>(ShaderStage::VERTEX)}
        }; // Set 0, Binding 0: GlobalUBO
        desc.ds_layouts_desc[1].bindings = {
            {1, DescriptorType::UNIFORM_BUFFER,  // ← OpenGL: binding=1 (not 0!)
             static_cast<uint32_t>(ShaderStage::VERTEX)}
        }; // Set 1, Binding 1: MaterialUBO (global binding space)

        desc.pl_desc.name = "default";
        desc.pl_desc.shader_modules = {
            {"res/shaders/v_default.glsl", ShaderStage::VERTEX},
            {"res/shaders/f_default.glsl", ShaderStage::FRAGMENT}};
        desc.pl_desc.vertex_layout = Vertex::getLayout();
        desc.pl_desc.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
      });

  // Uniform layout for material uniforms (Set 1)
  // Must match shader layout!
  gl_default_mat.uniform_layout = UniformLayout::createStd140({
      UniformLayout::Variable{
          .name = "color",
          .offset = 0,
          .size = 12,  // vec3 = 12 bytes
          .type = Core::UniformValue::Type::Vec3
      }
      // std140 automatically adds 4 bytes padding to 16 bytes
  });
  gl_default_mat.uniform_layout.computeLayout();

  registry.add(std::move(gl_default_mat));
}

void DefaultPipeline::addConfigToRegistry(PipelineConfigRegistry &registry, BackendType type) {
  if (type == BackendType::OpenGL)
    initOpenglMaterial(registry);
  if (type == BackendType::Vulkan)
    initVulkanMaterial(registry);
}


} // namespace Render
