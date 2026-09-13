#include "slang_compiler.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#ifdef SSME_DUMP_SLANG_OUTPUT
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#endif

#include "core/uniform_layout.h"
#include "utils/debug_assert.h"

namespace ssme {

namespace {
constexpr const char *kModuleExtension = ".slang";
}

bool SlangCompiler::init() {
  if (SLANG_FAILED(slang::createGlobalSession(m_global_session.writeRef()))) {
    debug_assert(false, "SlangCompiler: createGlobalSession failed");
    return false;
  }

  slang::TargetDesc targets[3];
  int t = 0;

  targets[t] = {};
  targets[t].format = SLANG_SPIRV;
  targets[t].profile = m_global_session->findProfile("spirv_1_5");
  m_target_formats[t] = SLANG_SPIRV;
  ++t;

  targets[t] = {};
  targets[t].format = SLANG_GLSL;
  targets[t].profile = m_global_session->findProfile("glsl_450");
  m_target_formats[t] = SLANG_GLSL;
  ++t;

  if (dxilAvailable()) {
    targets[t] = {};
    targets[t].format = SLANG_DXIL;
    targets[t].profile = m_global_session->findProfile("sm_6_0");
    m_target_formats[t] = SLANG_DXIL;
    ++t;
  }

  m_target_count = t;

  const char *search_paths[] = {"", "res/shaders"};

  slang::CompilerOptionValue opt_value{};
  opt_value.kind = slang::CompilerOptionValueKind::Int;
  opt_value.intValue0 = 1; // 1 = включить
  slang::CompilerOptionEntry opt_entry{};
  opt_entry.name = slang::CompilerOptionName::MatrixLayoutColumn;
  opt_entry.value = opt_value;

  slang::SessionDesc desc;

  desc.targets = targets;
  desc.targetCount = m_target_count;
  desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
  desc.compilerOptionEntries = &opt_entry;   // <-- новое
  desc.compilerOptionEntryCount = 1;          // <-- новое
  desc.searchPaths = search_paths;
  desc.searchPathCount = 2;

  if (SLANG_FAILED(
          m_global_session->createSession(desc, m_session.writeRef()))) {
    debug_assert(false, "SlangCompiler: createSession failed");
    return false;
  }
  return true;
}

bool SlangCompiler::compile(ShaderStage stage, const std::string &name,
                            ShaderModuleDesc &out) {
  if (!m_session) {
    debug_assert(false, "SlangCompiler: session is not initialized");
    return false;
  }

  out = ShaderModuleDesc{};
  out.file_path = name;
  out.stage = stage;

  std::string module_name = name;
  const size_t ext_len = std::string(kModuleExtension).size();
  if (module_name.size() > ext_len &&
      module_name.compare(module_name.size() - ext_len, ext_len,
                          kModuleExtension) == 0) {
    module_name.resize(module_name.size() - ext_len);
  }

  Slang::ComPtr<slang::IBlob> diag;

  Slang::ComPtr<slang::IModule> module(
      m_session->loadModule(module_name.c_str(), diag.writeRef()));
  if (!module) {
    logDiagnostics("loadModule(" + module_name + ")", diag);
    return false;
  }

  Slang::ComPtr<slang::IEntryPoint> entry_point;
  if (SLANG_FAILED(
          module->findEntryPointByName("main", entry_point.writeRef()))) {
    logDiagnostics("findEntryPointByName(main)", diag);
    return false;
  }

  slang::IComponentType *parts[2] = {module.get(), entry_point.get()};
  Slang::ComPtr<slang::IComponentType> program;
  if (SLANG_FAILED(m_session->createCompositeComponentType(
          parts, 2, program.writeRef(), diag.writeRef()))) {
    logDiagnostics("createCompositeComponentType", diag);
    return false;
  }

  Slang::ComPtr<slang::IComponentType> linked;
  if (SLANG_FAILED(program->link(linked.writeRef(), diag.writeRef()))) {
    logDiagnostics("link()", diag);
    return false;
  }

  for (int ti = 0; ti < m_target_count; ++ti) {
    Slang::ComPtr<slang::IBlob> code;
    if (SLANG_FAILED(linked->getEntryPointCode(0, ti, code.writeRef(),
                                               diag.writeRef()))) {
      logDiagnostics("getEntryPointCode(target " + std::to_string(ti) + ")",
                     diag);
      return false;
    }
    const char *begin = static_cast<const char *>(code->getBufferPointer());
    std::vector<char> bytes(begin, begin + code->getBufferSize());
    if (bytes.empty()) {
      logDiagnostics("empty code for target " + std::to_string(ti), diag);
      return false;
    }
    switch (m_target_formats[ti]) {
    case SLANG_SPIRV:
      out.code.spirv = std::move(bytes);
      break;
    case SLANG_GLSL:
      out.code.glsl = std::move(bytes);
      break;
    case SLANG_DXIL:
      out.code.dxil = std::move(bytes);
      break;
    default:
      break;
    }
  }

  // getLayout() is valid while `linked` stays alive; do reflection before it is
  // released.
  const bool ok = buildReflection(linked, stage, out);
#ifdef SSME_DUMP_SLANG_OUTPUT
  dumpArtifacts(out);
#endif
  return ok;

}

// ============================================================================
// Reflection
// ============================================================================

struct PendingBinding {
  uint32_t set = 0;
  uint32_t binding = 0;
  std::string name;
  std::string type_name;
  DescriptorType type = DescriptorType::UNIFORM_BUFFER;
  bool merged_away_sampler =
      false; // consumed by a combined image sampler merge
};

bool SlangCompiler::buildReflection(slang::IComponentType *program,
                                    ShaderStage stage, ShaderModuleDesc &out) {
  slang::ProgramLayout *layout =
      program->getLayout(0); // Vulkan (SPIR-V) target
  if (!layout) {
    debug_assert(false, "SlangCompiler: getLayout returned null");
    return false;
  }

  ShaderReflectionData &refl = out.reflection;
  const uint32_t stage_bits = static_cast<uint32_t>(stage);

  // ---- Varying inputs (vertex stage only) ----
  if (stage == ShaderStage::VERTEX) {
    slang::EntryPointReflection *entry = layout->getEntryPointByIndex(0);
    if (!entry) {
      debug_assert(false, "SlangCompiler: getEntryPointByIndex returned null");
      return false;
    }
    slang::VariableLayoutReflection *scope = entry->getVarLayout();
    slang::TypeLayoutReflection *scope_tl =
        scope ? scope->getTypeLayout() : nullptr;
    if (scope_tl &&
        scope_tl->getKind() == slang::TypeReflection::Kind::Struct) {
      for (unsigned i = 0; i < scope_tl->getFieldCount(); ++i) {
        slang::VariableLayoutReflection *field = scope_tl->getFieldByIndex(i);
        if (!field) {
          continue;
        }
        VertexInputRequirement req;
        req.location = static_cast<uint32_t>(
            field->getOffset(slang::ParameterCategory::VaryingInput));
        const char *semantic = field->getSemanticName();
        const char *var_name = field->getName();
        req.name = semantic ? semantic : (var_name ? var_name : "");
        req.expected_format = toVertexFormat(field->getTypeLayout());
        refl.vertex_requirements.push_back(std::move(req));
      }
    }
  }

  // ---- Global shader parameters ----
  slang::VariableLayoutReflection *global_scope =
      layout->getGlobalParamsVarLayout();
  if (!global_scope) {
    debug_assert(false,
                 "SlangCompiler: getGlobalParamsVarLayout returned null");
    return false;
  }
  slang::TypeLayoutReflection *global_tl = global_scope->getTypeLayout();
  // Unwrap auto-introduced ParameterBlock / ConstantBuffer scopes.
  while (
      global_tl &&
      (global_tl->getKind() == slang::TypeReflection::Kind::ParameterBlock ||
       global_tl->getKind() == slang::TypeReflection::Kind::ConstantBuffer)) {
    slang::VariableLayoutReflection *element = global_tl->getElementVarLayout();
    global_tl = element ? element->getTypeLayout() : nullptr;
  }
  if (!global_tl ||
      global_tl->getKind() != slang::TypeReflection::Kind::Struct) {
    debug_assert(false, "SlangCompiler: unexpected global scope layout");
    return false;
  }

  std::vector<PendingBinding> pending;
  std::map<std::pair<uint32_t, uint32_t>, size_t> texture_binding_index;

  for (unsigned i = 0; i < global_tl->getFieldCount(); ++i) {
    slang::VariableLayoutReflection *var = global_tl->getFieldByIndex(i);
    if (!var) {
      continue;
    }
    slang::TypeLayoutReflection *tl = var->getTypeLayout();
    if (!tl) {
      continue;
    }
    const char *var_name = var->getName();
    const std::string name = var_name ? var_name : "";

    // ---- Push constants ----
    if (hasCategory(var, slang::ParameterCategory::PushConstantBuffer)) {
      slang::TypeLayoutReflection *element =
          tl->getElementVarLayout() ? tl->getElementVarLayout()->getTypeLayout()
                                    : nullptr;
      PushConstantRange range;
      range.stages = stage_bits;
      range.offset = static_cast<uint32_t>(
          var->getOffset(slang::ParameterCategory::PushConstantBuffer));
      size_t size =
          element
              ? element->getSize(slang::ParameterCategory::PushConstantBuffer)
              : 0;
      if (element && size == 0) {
        size = element->getSize();
      }
      range.size = static_cast<uint32_t>(size);
      refl.push_constants[name] = range;
      if (element) {
        refl.push_constants_layouts[name] = buildStructLayout(element);
      }
      continue;
    }

    const slang::TypeReflection::Kind kind = tl->getKind();

    // ---- Uniform buffer (ConstantBuffer<T>) ----
    if (kind == slang::TypeReflection::Kind::ConstantBuffer) {
      slang::VariableLayoutReflection *element = tl->getElementVarLayout();
      slang::TypeLayoutReflection *element_tl =
          element ? element->getTypeLayout() : nullptr;
      if (!element || !element_tl) {
        debug_assert(false,
                     "SlangCompiler: ConstantBuffer without element layout");
        return false;
      }
      std::string type_name =
          element_tl->getName() ? element_tl->getName() : name;
      refl.binding_layouts[type_name] = buildStructLayout(element_tl);

      PendingBinding b;
      b.set = static_cast<uint32_t>(
          var->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot));
      b.binding = static_cast<uint32_t>(
          var->getOffset(slang::ParameterCategory::DescriptorTableSlot));
      b.name = name;
      b.type_name = type_name;
      b.type = DescriptorType::UNIFORM_BUFFER;
      pending.push_back(std::move(b));
      continue;
    }

    // ---- Texture (Texture2D etc.) ----
    if (kind == slang::TypeReflection::Kind::Resource) {
      PendingBinding b;
      b.set = static_cast<uint32_t>(
          var->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot));
      b.binding = static_cast<uint32_t>(
          var->getOffset(slang::ParameterCategory::DescriptorTableSlot));
      b.name = name;
      b.type = DescriptorType::SAMPLED_IMAGE;
      pending.push_back(b);
      texture_binding_index[{b.set, b.binding}] = pending.size() - 1;
      continue;
    }

    // ---- Sampler (SamplerState) ----
    if (kind == slang::TypeReflection::Kind::SamplerState) {
      PendingBinding b;
      b.set = static_cast<uint32_t>(
          var->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot));
      b.binding = static_cast<uint32_t>(
          var->getOffset(slang::ParameterCategory::DescriptorTableSlot));
      b.name = name;
      b.type = DescriptorType::SAMPLER;

      // A sampler bound to the same (set, binding) as a texture merges into one
      // combined image sampler (matches Hoops/Slang SPIR-V output and the
      // engine).
      auto it = texture_binding_index.find({b.set, b.binding});
      if (it != texture_binding_index.end()) {
        pending[it->second].type = DescriptorType::COMBINED_IMAGE_SAMPLER;
        b.merged_away_sampler = true;
      }
      pending.push_back(std::move(b));
      continue;
    }
  }

  // ---- Assemble descriptor set layouts ----
  for (const auto &p : pending) {
    if (p.merged_away_sampler) {
      continue;
    }
    Binding dsl;
    dsl.set = p.set;
    dsl.binding = p.binding;
    dsl.name = p.name;
    dsl.type_name = p.type_name;
    dsl.type = p.type;
    dsl.count = 1;
    dsl.stages = stage_bits;
    refl.ds_layouts[p.set].bindings.push_back(std::move(dsl));
    refl.descriptor_set_count = std::max(refl.descriptor_set_count, p.set + 1);
  }
  refl.required_components = refl.descriptor_set_count + 1;

  return true;
}

// ============================================================================
// Helpers
// ============================================================================

std::shared_ptr<UniformLayout>
SlangCompiler::buildStructLayout(slang::TypeLayoutReflection *structTl) {
  auto layout = std::make_shared<UniformLayout>();
  if (!structTl || structTl->getKind() != slang::TypeReflection::Kind::Struct) {
    return layout;
  }
  for (unsigned i = 0; i < structTl->getFieldCount(); ++i) {
    slang::VariableLayoutReflection *field = structTl->getFieldByIndex(i);
    if (!field) {
      continue;
    }
    const char *field_name = field->getName();
    if (!field_name) {
      continue;
    }
    // Offsets are computed by UniformLayout::computeLayout() (CPU std140) and
    // must match the shader's std140 layout, so no manual offsets are passed
    // here.
    layout->addVariable(field_name, toUniformType(field->getTypeLayout()));
  }
  layout->computeLayout();
  return layout;
}

Format SlangCompiler::toVertexFormat(slang::TypeLayoutReflection *typeLayout) {
  slang::TypeReflection *type = typeLayout ? typeLayout->getType() : nullptr;
  if (!type) {
    return Format::UNDEFINED;
  }
  int components = 1;
  if (type->getKind() == slang::TypeReflection::Kind::Vector) {
    components = static_cast<int>(type->getElementCount());
    type = type->getElementType();
  }
  if (!type ||
      type->getScalarType() != slang::TypeReflection::ScalarType::Float32) {
    return Format::UNDEFINED;
  }
  switch (components) {
  case 2:
    return Format::R32G32_SFLOAT;
  case 3:
    return Format::R32G32B32_SFLOAT;
  case 4:
    return Format::R32G32B32A32_SFLOAT;
  default:
    return Format::UNDEFINED;
  }
}

bool SlangCompiler::hasCategory(slang::VariableLayoutReflection *varLayout,
                                slang::ParameterCategory category) {
  if (!varLayout) {
    return false;
  }
  for (unsigned i = 0; i < varLayout->getCategoryCount(); ++i) {
    if (varLayout->getCategoryByIndex(i) == category) {
      return true;
    }
  }
  return false;
}

UniformValue::Type
SlangCompiler::toUniformType(slang::TypeLayoutReflection *typeLayout) {
  slang::TypeReflection *type = typeLayout ? typeLayout->getType() : nullptr;
  if (!type) {
    return UniformValue::Type::Float;
  }
  const slang::TypeReflection::Kind kind = type->getKind();
  if (kind == slang::TypeReflection::Kind::Scalar) {
    switch (type->getScalarType()) {
    case slang::TypeReflection::ScalarType::Int32:
      return UniformValue::Type::Int;
    case slang::TypeReflection::ScalarType::UInt32:
      return UniformValue::Type::Uint;
    case slang::TypeReflection::ScalarType::Bool:
      return UniformValue::Type::Bool;
    case slang::TypeReflection::ScalarType::Float64:
      return UniformValue::Type::Double;
    case slang::TypeReflection::ScalarType::Float32:
    default:
      return UniformValue::Type::Float;
    }
  }
  if (kind == slang::TypeReflection::Kind::Vector) {
    const int n = static_cast<int>(type->getElementCount());
    const auto scalar = type->getElementType()->getScalarType();
    if (scalar == slang::TypeReflection::ScalarType::Int32) {
      switch (n) {
      case 2:
        return UniformValue::Type::IVec2;
      case 3:
        return UniformValue::Type::IVec3;
      default:
        return UniformValue::Type::IVec4;
      }
    }
    if (scalar == slang::TypeReflection::ScalarType::UInt32) {
      switch (n) {
      case 2:
        return UniformValue::Type::UVec2;
      case 3:
        return UniformValue::Type::UVec3;
      default:
        return UniformValue::Type::UVec4;
      }
    }
    switch (n) {
    case 2:
      return UniformValue::Type::Vec2;
    case 3:
      return UniformValue::Type::Vec3;
    default:
      return UniformValue::Type::Vec4;
    }
  }
  if (kind == slang::TypeReflection::Kind::Matrix) {
    return UniformValue::Type::Mat4; // current shaders use float4x4 only
  }
  return UniformValue::Type::Float;
}

void SlangCompiler::logDiagnostics(const std::string &what,
                                   slang::IBlob *diag) const {
  std::string msg = "SlangCompiler: " + what + " failed";
  if (diag && diag->getBufferSize() > 0) {
    msg += "\n";
    msg.append(static_cast<const char *>(diag->getBufferPointer()),
               diag->getBufferSize());
  }
  debug_assert(false, msg);
}

bool SlangCompiler::dxilAvailable() {
  slang::TargetDesc td = {};
  td.format = SLANG_DXIL;
  td.profile = m_global_session->findProfile("sm_6_0");
  slang::SessionDesc sd = {};
  sd.targets = &td;
  sd.targetCount = 1;

  Slang::ComPtr<slang::ISession> probe_session;
  if (SLANG_FAILED(
          m_global_session->createSession(sd, probe_session.writeRef())))
    return false;

  const char *src =
      "[shader(\"compute\")]\n[numthreads(1,1,1)]\nvoid main() {}\n";
  Slang::ComPtr<slang::IBlob> diag;
  Slang::ComPtr<slang::IModule> mod(probe_session->loadModuleFromSourceString(
      "__dxil_probe__", "__dxil_probe__.slang", src, diag.writeRef()));
  if (!mod)
    return false;

  Slang::ComPtr<slang::IEntryPoint> ep;
  if (SLANG_FAILED(mod->findEntryPointByName("main", ep.writeRef())))
    return false;

  slang::IComponentType *parts[2] = {mod.get(), ep.get()};
  Slang::ComPtr<slang::IComponentType> program;
  if (SLANG_FAILED(probe_session->createCompositeComponentType(
          parts, 2, program.writeRef(), diag.writeRef())))
    return false;

  Slang::ComPtr<slang::IComponentType> linked;
  if (SLANG_FAILED(program->link(linked.writeRef(), diag.writeRef())))
    return false;

  Slang::ComPtr<slang::IBlob> code;
  return SLANG_SUCCEEDED(
      linked->getEntryPointCode(0, 0, code.writeRef(), diag.writeRef()));
}

// ============================================================================
// Debug dump (guarded by SSME_DUMP_SLANG_OUTPUT)
// ============================================================================

#ifdef SSME_DUMP_SLANG_OUTPUT

namespace {

const char *descriptorTypeName(DescriptorType type) {
  switch (type) {
  case DescriptorType::SAMPLER: return "SAMPLER";
  case DescriptorType::COMBINED_IMAGE_SAMPLER: return "COMBINED_IMAGE_SAMPLER";
  case DescriptorType::SAMPLED_IMAGE: return "SAMPLED_IMAGE";
  case DescriptorType::STORAGE_IMAGE: return "STORAGE_IMAGE";
  case DescriptorType::UNIFORM_BUFFER: return "UNIFORM_BUFFER";
  case DescriptorType::STORAGE_BUFFER: return "STORAGE_BUFFER";
  }
  return "UNKNOWN";
}

const char *formatName(Format format) {
  switch (format) {
  case Format::UNDEFINED: return "UNDEFINED";
  case Format::R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
  case Format::R32G32B32_SFLOAT: return "R32G32B32_SFLOAT";
  case Format::R32G32_SFLOAT: return "R32G32_SFLOAT";
  case Format::R32_SFLOAT: return "R32_SFLOAT";
  case Format::R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
  case Format::R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
  case Format::D32F: return "D32F";
  }
  return "UNKNOWN";
}

const char *stageName(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::VERTEX: return "vertex";
  case ShaderStage::FRAGMENT: return "fragment";
  case ShaderStage::COMPUTE: return "compute";
  }
  return "unknown";
}

const char *uniformTypeName(UniformValue::Type type) {
  switch (type) {
  case UniformValue::Type::Float: return "float";
  case UniformValue::Type::Int: return "int";
  case UniformValue::Type::Uint: return "uint";
  case UniformValue::Type::Bool: return "bool";
  case UniformValue::Type::Double: return "double";
  case UniformValue::Type::Vec2: return "vec2";
  case UniformValue::Type::Vec3: return "vec3";
  case UniformValue::Type::Vec4: return "vec4";
  case UniformValue::Type::IVec2: return "ivec2";
  case UniformValue::Type::IVec3: return "ivec3";
  case UniformValue::Type::IVec4: return "ivec4";
  case UniformValue::Type::UVec2: return "uvec2";
  case UniformValue::Type::UVec3: return "uvec3";
  case UniformValue::Type::UVec4: return "uvec4";
  case UniformValue::Type::Mat4: return "mat4";
  }
  return "unknown";
}

} // namespace

void SlangCompiler::dumpArtifacts(const ShaderModuleDesc &out) const {
  namespace fs = std::filesystem;

  const fs::path dump_dir = fs::path("res") / "shaders" / "dump";
  std::error_code ec;
  fs::create_directories(dump_dir, ec);

  const fs::path base = dump_dir / out.file_path;

  if (!out.code.glsl.empty()) {
    const std::string text(out.code.glsl.data(), out.code.glsl.size());
    std::ofstream(base.string() + ".glsl") << text;
  }
  if (!out.code.spirv.empty()) {
    std::ofstream f(base.string() + ".spv", std::ios::binary);
    f.write(out.code.spirv.data(),
            static_cast<std::streamsize>(out.code.spirv.size()));
  }
  if (!out.code.dxil.empty()) {
    std::ofstream f(base.string() + ".dxil", std::ios::binary);
    f.write(out.code.dxil.data(),
            static_cast<std::streamsize>(out.code.dxil.size()));
  }

  std::ostringstream s;
  s << "shader: " << out.file_path << "\n";
  s << "stage: " << stageName(out.stage) << "\n\n";

  const ShaderReflectionData &refl = out.reflection;

  s << "-- vertex inputs --\n";
  if (refl.vertex_requirements.empty()) {
    s << "(none)\n";
  }
  for (const auto &req : refl.vertex_requirements) {
    s << "location " << req.location << ": " << req.name << " ("
      << formatName(req.expected_format) << ")\n";
  }

  s << "\n-- push constants --\n";
  if (refl.push_constants.empty()) {
    s << "(none)\n";
  }
  for (const auto &[name, range] : refl.push_constants) {
    s << name << ": offset " << range.offset << ", size " << range.size
      << ", stages 0x" << std::hex << range.stages << std::dec << "\n";
  }

  s << "\n-- descriptor sets --\n";
  if (refl.ds_layouts.empty()) {
    s << "(none)\n";
  }
  for (const auto &[set, layout] : refl.ds_layouts) {
    s << "set " << set << ":\n";
    for (const auto &b : layout.bindings) {
      s << "  binding " << b.binding << ": [" << descriptorTypeName(b.type)
        << "] " << b.name;
      if (b.type_name.empty() || b.type_name == b.name) {
        s << "\n";
      } else {
        s << " (" << b.type_name << ")\n";
      }
    }
  }

  s << "\n-- binding layouts (std140) --\n";
  if (refl.binding_layouts.empty()) {
    s << "(none)\n";
  }
  for (const auto &[name, layout] : refl.binding_layouts) {
    s << name << " (total " << layout->getTotalSize() << " bytes):\n";
    for (const auto &v : layout->getVariables()) {
      s << "  " << v.name << " offset " << v.offset << ", size " << v.size
        << " (" << uniformTypeName(v.type) << ")\n";
    }
  }

  s << "\ndescriptor_set_count: " << refl.descriptor_set_count << "\n";
  s << "required_components: " << refl.required_components << "\n";

  std::ofstream(base.string() + ".txt") << s.str();
}

#endif // SSME_DUMP_SLANG_OUTPUT

} // namespace ssme
