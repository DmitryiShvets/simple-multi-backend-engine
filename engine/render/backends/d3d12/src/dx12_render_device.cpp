#include "dx12_render_device.h"
#include "core/resource_types.h"
#include "core/rid.h"
#include "dx12_descriptor_set.h"
#include "dx12_device.h"
#include "dx12_gpu_storage.h"
#include "dx12_helpers.h"
#include "dx12_pipeline.h"
#include "dx12_pipeline_layout.h"
#include "dx12_shader_module.h"
#include "utils/debug_assert.h"
#include <d3d12.h>
#include <d3dx12.h>

#include <cstddef>
#include <iostream>

namespace ssme::d3d12 {

Dx12RenderDevice::Dx12RenderDevice(Dx12Device &device,
                                   Dx12GpuStorageMT &storage)
    : m_device(device), m_storage(storage) {}

Dx12RenderDevice::~Dx12RenderDevice() {}

//------------------------------------------------------------------------
// ---------------------------- Buffer -----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createBuffer(const BufferDesc &desc, RID id) {
  debug_assert(desc.size > 0, "Buffer size cannot be zero");
  auto elment_size = desc.element_size;
  debug_assert(elment_size > 0, "Buffer must have a non-zero stride");
  uint64_t elements_count = desc.size / elment_size;
  debug_assert(elements_count > 0, "Buffer size cannot empty");
  D3D12_HEAP_TYPE heapType =
      desc.is_host_visible ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_STATES initialState = desc.is_host_visible
                                           ? D3D12_RESOURCE_STATE_GENERIC_READ
                                           : D3D12_RESOURCE_STATE_COMMON;
  auto buffer = std::make_unique<Dx12Buffer>(
      m_device, elment_size, elements_count, heapType, initialState);
  debug_assert(buffer != nullptr,
               "Failed to allocate VulkanBuffer (Out of memory?)");
  if (desc.initial_data) {
    buffer->map();
    buffer->writeToBuffer(desc.initial_data);
    buffer->unmap();
  }
  if (id.isNull()) {
    id = m_storage.add(std::move(buffer));
  } else {
    m_storage.store(id, std::move(buffer));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void Dx12RenderDevice::destroyBuffer(RID rid) {
  if (rid.isNull())
    return;

  auto *buffer = m_storage.get<Dx12Buffer>(rid);
  if (buffer) {
    // Remove from storage (this will call VulkanBuffer destructor)
    m_storage.remove<Dx12Buffer>(rid);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Texture ----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createTexture(const TextureDesc &desc, RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyTexture(RID rid) {
  if (rid.isNull())
    return;
  // TODO: Implement when textures are added
}

//------------------------------------------------------------------------
// ---------------------------- Sampler ----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createSampler(const SamplerDesc &desc, RID id) {
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor layout ------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createDescriptorLayout(const DescriptorLayout &desc,
                                             RID id) {
  auto layout = std::make_unique<Dx12DescriptorSetLayout>(m_device, desc);
  if (id.isNull()) {
    id = m_storage.add(std::move(layout));
  } else {
    m_storage.store(id, std::move(layout));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void Dx12RenderDevice::destroyDescriptorLayout(RID id) {
  if (id.isNull())
    return;
  auto *ds_layout = m_storage.get<Dx12DescriptorSetLayout>(id);
  if (ds_layout) {
    // Remove from storage (this will call Dx12DescriptorSetLayout destructor)
    m_storage.remove<Dx12DescriptorSetLayout>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor -------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createDescriptor(const DescriptorDesc &desc, RID id) {
  auto ds = std::make_unique<Dx12DescriptorSet>(desc);
  if (id.isNull()) {
    id = m_storage.add(std::move(ds));
  } else {
    m_storage.store(id, std::move(ds));
  }
  return id;
}

void Dx12RenderDevice::destroyDescriptor(RID id) {
  if (id.isNull())
    return;

  auto *layout = m_storage.get<Dx12DescriptorSet>(id);
  if (layout) {
    // Remove from storage (this will call Dx12DescriptorSet destructor)
    m_storage.remove<Dx12DescriptorSet>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline layout --------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createPipelineLayout(const PipelineLayoutDesc &desc,
                                           RID id) {
  std::size_t hash = desc.hash();
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_storage.findPSOLayout(hash);
  if (existing_rid) {
    debug_assert(
        false, "External error in Resource manager. Atempt to create Pipeline "
               "layout that already "
               "existed. Resource manager is not found by PipelimeParams HASH");
    // return existing_rid;
  }
  std::vector<CD3DX12_ROOT_PARAMETER> params;
  // collect uniforms
  for (const RID &id : desc.descriptor_layouts) {
    auto *ds_layout = m_storage.get<Dx12DescriptorSetLayout>(id);
    if (ds_layout) {
      for (auto &binding : ds_layout->getLayout().bindings) {
        if (binding.type == DescriptorType::UNIFORM_BUFFER) {
          CD3DX12_ROOT_PARAMETER p;
          p.InitAsConstantBufferView(binding.binding, binding.set);
          params.push_back(p);
        }
        // TODO: SRV, UAV, Sampler — когда понадобятся
      }
    } else {
      // Handle error: invalid RID
      throw std::runtime_error(
          "Invalid descriptor set layout RID in createPipelineLayout");
    }
  }
  // collect push constants
  const auto push_constant_start_index = params.size();
  for (auto &pc : desc.push_constant_ranges) {
    CD3DX12_ROOT_PARAMETER p;
    p.InitAsConstants(pc.size / 4, 0, 3); // DWORDs, reg=0, space=3
    params.push_back(p);
  }

  auto layout_wrapper = std::move(std::make_unique<Dx12PipelineLayout>(
      m_device, params, push_constant_start_index));

  if (id.isNull()) {
    id = m_storage.add(std::move(layout_wrapper));
  } else {
    m_storage.store(id, std::move(layout_wrapper));
  }

  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  m_storage.registerPSOLayout(hash, id);

  return id;
}

void Dx12RenderDevice::destroyPipelineLayout(RID id) {
  if (id.isNull())
    return;

  auto *layout = m_storage.get<Dx12PipelineLayout>(id);
  if (layout) {
    // Remove from storage (this will call Dx12PipelineLayout destructor)
    m_storage.remove<Dx12PipelineLayout>(id);
  }
}

RID Dx12RenderDevice::containsPipelineLayout(std::size_t hash) {
  RID existing_rid = m_storage.findPSOLayout(hash);
  if (existing_rid) {
    return existing_rid;
  }
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline ---------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createGraphicsPipeline(const GraphicsPipelineDesc &desc,
                                             RID id) {
  std::size_t hash = desc.hash();
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_storage.findPSO(hash);
  if (existing_rid) {
    debug_assert(
        false, "External error in Resource manager. Atept to create PSO that "
               "already "
               "existed. Resource manager is not found by PipelimeParams HASH");
    // return existing_rid;
  }
  // --- If not found, create a new one ---
  // 2. Get the pre-created pipeline layout from the resource manager
  auto dx_pl_layout = m_storage.get<Dx12PipelineLayout>(desc.pl_layout_id);
  if (!dx_pl_layout) {
    throw std::runtime_error("Invalid pipeline layout RID in createPipeline");
  }
  auto dx_pipeline_layout = dx_pl_layout->getHandle();
  if (!dx_pipeline_layout) {
    throw std::runtime_error("createGraphicsPipeline: pipeline_layout is null");
  }
  // 3. Translate abstract desc to DirectX-specific PipelineConfigInfo
  // this data describes how to pass this data format to the vertex shader
  // Input layout из VertexLayout
  std::vector<D3D12_INPUT_ELEMENT_DESC> input_elements;
  for (auto &attr : desc.vertex_layout.getAttributes()) {
    input_elements.push_back({
        .SemanticName = "ATTRIB",            // SemanticName — захардкожено
        .SemanticIndex = attr.location,      // SemanticIndex
        .Format = toDxgiFormat(attr.format), // Format
        .InputSlot = attr.binding,           // InputSlot
        .AlignedByteOffset = attr.offset,    // AlignedByteOffset
        .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        .InstanceDataStepRate = 0,
    });
  }
  // 4. Find shaders paths from the abstract descriptor
  auto vert_shader = m_storage.get<Dx12ShaderModule>(desc.vert_shader_module);
  auto frag_shader = m_storage.get<Dx12ShaderModule>(desc.frag_shader_module);
  if (!vert_shader || !frag_shader) {
    throw std::runtime_error(
        "Vertex and Fragment shader paths must be provided");
  }
  D3D12_RASTERIZER_DESC rasteraizer_desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  rasteraizer_desc.FrontCounterClockwise = TRUE;
  // 5. Create the VulkanPipeLine object
  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{
      .pRootSignature = dx_pipeline_layout.Get(),
      .VS = vert_shader->getBytecode(),
      .PS = frag_shader->getBytecode(),
      .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
      .SampleMask = UINT_MAX,
      .RasterizerState = rasteraizer_desc,
      .DepthStencilState =
          {
              .DepthEnable = FALSE,
              .StencilEnable = FALSE,
          },
      .InputLayout = {input_elements.data(), (UINT)input_elements.size()},
      .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
      .NumRenderTargets = 1,
      .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
      .SampleDesc = {1, 0},
  };
  auto pipeline = std::make_unique<Dx12Pipeline>(m_device, dx_pipeline_layout,
                                                 pso_desc, desc.pl_layout_id);

  // 6. Store it in the resource manager and register its name
  if (id.isNull()) {
    id = m_storage.add(std::move(pipeline));
  } else {
    m_storage.store(id, std::move(pipeline));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  m_storage.registerPSO(hash, id);

  return id;
}

void Dx12RenderDevice::destroyGraphicsPipeline(RID id) {
  if (id.isNull())
    return;

  auto *pipeline = m_storage.get<Dx12Pipeline>(id);
  if (pipeline) {
    // Remove from storage (this will call Dx12Pipeline destructor)
    m_storage.remove<Dx12Pipeline>(id);
  }
}

RID Dx12RenderDevice::containsGraphicsPipeline(std::size_t hash) {
  RID existing_rid = m_storage.findPSO(hash);
  if (existing_rid) {
    return existing_rid;
  }
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Shader -----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createShaderModule(const ShaderModuleDesc &desc, RID id) {
  auto path = "res/shaders/" + desc.file_path + ".hlsl.cso";
  auto shader_module = std::make_unique<Dx12ShaderModule>(m_device, path);
  if (id.isNull()) {
    id = m_storage.add(std::move(shader_module));
  } else {
    m_storage.store(id, std::move(shader_module));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void Dx12RenderDevice::destroyShaderModule(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *shader_module = m_storage.get<Dx12ShaderModule>(id);
  if (shader_module) {
    // Remove from storage (this will call Dx12ShaderModule destructor)
    m_storage.remove<Dx12ShaderModule>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Common -----------------------------------
// -----------------------------------------------------------------------

void Dx12RenderDevice::updateBufferRaw(RID rid, size_t offset, size_t size,
                                       const void *data) {
  auto *buffer = m_storage.get<Dx12Buffer>(rid);
  if (!buffer)
    return;
  buffer->map(size, offset);
  buffer->writeToBuffer(data, size, offset);
  buffer->unmap();
}

} // namespace ssme::d3d12
