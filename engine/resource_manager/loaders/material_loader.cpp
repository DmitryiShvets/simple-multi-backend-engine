#include "material_loader.h"
#include "resource_manager.h"
#include "resources/material.h"
#include "resources/shader.h"
#include "resources/uniform_block.h"
#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

namespace ssme {

using json = nlohmann::json;

bool MaterialLoader::load(const std::string &path, ResourceManager &rm,
                          void *out_params) {
  std::ifstream file("res/materials/" + path);
  if (!file.is_open())
    return false;

  json data = json::parse(file);
  auto *params = static_cast<MaterialParams *>(out_params);

  // 1. RECURSION: Load shaders through RM
  // If already cached — RM returns immediately. If not — calls ShaderLoader.
  params->vert_shader =
      rm.load<ShaderModule>(data["vertex_shader"].get<std::string>());
  params->frag_shader =
      rm.load<ShaderModule>(data["fragment_shader"].get<std::string>());

  // 2. Need access to shader to create proper UniformSet
  ShaderModule *shader = nullptr;

  // 3. Parse data blocks (UBO) from JSON
  if (data.contains("blocks")) {
    for (auto &[block_name, vars] : data["blocks"].items()) {
      if (params->vert_shader->containsUniformBlock(block_name)) {
        shader = params->vert_shader.get();
      } else if (params->frag_shader->containsUniformBlock(block_name)) {
        shader = params->frag_shader.get();
      }
      if (!shader)
        return false;

      // Create "blockblock" based on shader reflection
      auto u_set = shader->createUniformSet(block_name);

      for (auto &[var_name, var_val] : vars.items()) {
        if (var_val.is_number()) {
          u_set.setFloat(var_name, var_val.get<float>());
        } else if (var_val.is_array() && var_val.size() == 3) {
          u_set.setVec3(var_name,
                        glm::vec3(var_val[0], var_val[1], var_val[2]));
        }
        // Can add vec4, bool, etc. here
      }

      auto layout = shader->getLayout(block_name);
      std::vector<uint8_t> data = layout->pack(u_set);

      UniformBlockDesc u_desc{
          .name = block_name,
          .size = data.size(),
          .data = data,
          .layout = layout,
      };

      params->uniforms[block_name] =
          rm.load<UniformBuffer>(path + block_name, u_desc);
      params->blocks_data[block_name] = std::move(u_set);
    }
  }

  // 4. Textures (if any)
  if (data.contains("textures")) {
    for (auto &[slot, tex_path] : data["textures"].items()) {
      params->textures[slot] = rm.load<Texture>(tex_path.get<std::string>());
    }
  }

  return true;
}
} // namespace ssme
