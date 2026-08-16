#include "shader_loader.h"
#include "core/render_types.h"
#include "utils/common_utils.h"

namespace ssme {

bool ShaderLoader::load(const std::string &path, ResourceManager &rm, void *out_params) {
  auto *desc = static_cast<ShaderModuleDesc *>(out_params);
  desc->file_path = path;
  if (path[0] == 'v') {
      desc->stage = ShaderStage::VERTEX;
  } else if (path[0] == 'f') {
      desc->stage = ShaderStage::FRAGMENT;
  }
  auto &refl = desc->reflection;

  // 1. Read binary
  std::vector<char> code = CUtils::readFileChar("res/shaders/" + path + ".glsl.spv");
  spv_reflect::ShaderModule spv_module{code.size(), code.data()};
  if (spv_module.GetResult() != SPV_REFLECT_RESULT_SUCCESS) {
    debug_assert(false, "ERROR: could not process '" + path +
                            "' (is it a valid SPIR-V bytecode?)");
    return false;
  }

  // 2. Vertex Input reflection (simplified)
  if (spv_module.GetShaderStage() == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
    uint32_t input_count = 0;
    spv_module.EnumerateInputVariables(&input_count, nullptr);
    std::vector<SpvReflectInterfaceVariable *> inputs(input_count);
    spv_module.EnumerateInputVariables(&input_count, inputs.data());

    for (auto *input : inputs) {
      VertexInputRequirement req;
      req.location = input->location;
      req.name = input->name ? input->name : "";
      req.expected_format = toVertexFormat(input->format);
      refl.vertex_requirements.push_back(req);
    }
  }

  // 2.a Get push constant block count (usually 1 per stage)
  uint32_t push_block_count = 0;
  spv_module.EnumeratePushConstantBlocks(&push_block_count, nullptr);
  // 2.b Get actual push constant blocks
  std::vector<SpvReflectBlockVariable *> push_blocks(push_block_count);
  spv_module.EnumeratePushConstantBlocks(&push_block_count, push_blocks.data());
  // 2.c Fill our data
  for (auto *block : push_blocks) {
    PushConstantRange range;
    range.offset = block->offset;
    range.size = block->size;
    range.stages = (uint32_t)spv_module.GetShaderStage();

    // Save to ShaderModule
    refl.push_constants[block->name] = range;

    // Also create UniformLayout for this block
    // to conveniently fill with bytes on CPU side
    auto layout = std::make_shared<UniformLayout>();
    for (uint32_t i = 0; i < block->member_count; ++i) {
      auto &m = block->members[i];
      // can get size, padded_size from m
      layout->addVariable(m.name, toUniformValueType(m.type_description),
                          m.offset);
    }
    layout->computeLayout();
    refl.push_constants_layouts[block->name] = std::move(layout);
  }

  // 3. Collect Bindings (Textures, Buffers)
  uint32_t uniforms_block_count = 0;
  spv_module.EnumerateDescriptorSets(&uniforms_block_count, nullptr);
  std::vector<SpvReflectDescriptorSet *> spv_uniforms(uniforms_block_count);
  spv_module.EnumerateDescriptorSets(&uniforms_block_count,
                                     spv_uniforms.data());

  for (auto *uniform : spv_uniforms) {
    DescriptorLayout layout; // ONE layout per set
    refl.descriptor_set_count++;
    refl.required_components++;

    // Collect ALL bindings within this set
    for (size_t i = 0; i < uniform->binding_count; i++) {
      auto *spv_binding = uniform->bindings[i];
      Binding info;
      info.set = spv_binding->set;
      info.binding = spv_binding->binding;
      info.name = spv_binding->name;
      info.name = spv_binding->name ? spv_binding->name : "";
      info.type_name = "";
      if (spv_binding->type_description && spv_binding->type_description->type_name) {
        info.type_name = spv_binding->type_description->type_name;
      }
      info.stages = (uint32_t)spv_module.GetShaderStage();
      info.type = toDescriptorType(spv_binding->descriptor_type);
      info.count = spv_binding->count;
      layout.bindings.push_back(info);

      // If Uniform Block — create UniformLayout for it
      if (spv_binding->descriptor_type ==
          SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        auto uniform_layout = std::make_shared<UniformLayout>();
        for (uint32_t i = 0; i < spv_binding->block.member_count; ++i) {
          auto &m = spv_binding->block.members[i];
          // Map SPIR-V types to UniformValue::Type
          uniform_layout->addVariable(
              m.name, toUniformValueType(m.type_description), m.offset);
        }
        uniform_layout->computeLayout();
        refl.binding_layouts[info.type_name] = std::move(uniform_layout);
      }
    }
    refl.ds_layouts[uniform->set] = layout;
  }

  refl.required_components =
      refl.descriptor_set_count + 1; // +1 for Shader Module RID
  return true;
}

UniformValue::Type toUniformValueType(SpvReflectTypeDescription *type_desc) {
  if (!type_desc)
    return UniformValue::Type::Unknown;

  // Check type via numeric traits
  const auto &numeric = type_desc->traits.numeric;

  // Scalars
  if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL) {
    return UniformValue::Type::Bool;
  }

  if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
    if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
      switch (numeric.matrix.column_count) {
      case 2:
        return UniformValue::Type::Mat2;
      case 3:
        return UniformValue::Type::Mat3;
      case 4:
        return UniformValue::Type::Mat4;
      default:
        return UniformValue::Type::Unknown;
      }
    }

    if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
      switch (numeric.vector.component_count) {
      case 2:
        return UniformValue::Type::Vec2;
      case 3:
        return UniformValue::Type::Vec3;
      case 4:
        return UniformValue::Type::Vec4;
      default:
        return UniformValue::Type::Float;
      }
    }

    return UniformValue::Type::Float;
  }

  if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
    if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
      switch (numeric.vector.component_count) {
      case 2:
        return UniformValue::Type::IVec2;
      case 3:
        return UniformValue::Type::IVec3;
      case 4:
        return UniformValue::Type::IVec4;
      default:
        return UniformValue::Type::Int;
      }
    }
    return UniformValue::Type::Int;
  }
  return UniformValue::Type::Unknown;
}

DescriptorType toDescriptorType(SpvReflectDescriptorType type) {
  switch (type) {
  case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
    return DescriptorType::SAMPLER;

  case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    return DescriptorType::COMBINED_IMAGE_SAMPLER;

  case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    return DescriptorType::SAMPLED_IMAGE;

  case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    return DescriptorType::STORAGE_IMAGE;

  case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    return DescriptorType::UNIFORM_BUFFER; // Or separate type?

  case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    return DescriptorType::STORAGE_BUFFER; // Or separate type?

  case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    return DescriptorType::UNIFORM_BUFFER;

  case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    return DescriptorType::STORAGE_BUFFER;

  case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    return DescriptorType::UNIFORM_BUFFER;

  case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    return DescriptorType::STORAGE_BUFFER;

  case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    return DescriptorType::SAMPLED_IMAGE; // Subpass input

  case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    // Not in our enum, need to add or return default
    return DescriptorType::STORAGE_BUFFER;

  default:
    return DescriptorType::UNIFORM_BUFFER;
  }
}

Format toVertexFormat(SpvReflectFormat format) {
  switch (format) {
  case SPV_REFLECT_FORMAT_R32_SFLOAT:
    return Format::R32_SFLOAT;
  case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
    return Format::R32G32_SFLOAT;
  case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
    return Format::R32G32B32_SFLOAT;
  case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
    return Format::R32G32B32A32_SFLOAT;
  // ... other formats
  default:
    return Format::UNDEFINED;
  }
}
} // namespace ssme
