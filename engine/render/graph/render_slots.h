// godrays_slot.h
#pragma once
#include "core/render_types.h"
#include "core/rid.h"
#include "graph/render_graph.h"
#include "graph/render_gtaph_utils.h"
#include "graph/render_slot.h"

namespace ssme {

class SinkSlot : public RenderSlot {
public:
  std::string_view name() const override { return "SINK"; }

  void setSwapchain(RID color, Extent2D extent) {
    m_color_rid = color;
    m_extent = extent;
  }

  void onSetup(RenderGraph &graph) override {
    ResourceDesc swapchain_color_desc{
        .width = m_extent.width,
        .height = m_extent.height,
        .format = ResourceFormat::RGBA8_SRGB,
    };
    m_swapchain_color = graph.importResource(
        "swapchain/color", swapchain_color_desc, ResourceState::UNDEFINED);
    graph.bindImport(m_swapchain_color, m_color_rid);
  }
  void onResolve(RenderGraph &graph) override {
    m_final_color = graph.getResource("scene_color");
  }

  void onBuild(RenderGraph &graph) override {
    graph.addPass("Present")
        .read(m_final_color, ResourceState::TRANSFER_SRC)
        .write(m_swapchain_color, ResourceState::TRANSFER_DST)
        .setLoadOp(LoadOp::LOAD)
        .setExecuteCallback([this, &graph](CommandList &cmd) {
            cmd.blitImage(graph.ridOf(m_final_color),
                          graph.ridOf(m_swapchain_color),
                          ImageBlit{0, 0, m_extent.width, m_extent.height, 0, 0});
        });
  }

private:
  ResourceView m_swapchain_color, m_swapchain_depth;
  ResourceView m_final_color;
  RID m_color_rid, m_depth_rid;
  Extent2D m_extent;
};

class MainOpaqueSlot : public RenderSlot {
public:
  std::string_view name() const override { return "MAIN_OPAQUE"; }

  void onSetup(RenderGraph &graph) override {
    ResourceDesc scene_color_desc{
        .width = m_extent.width,
        .height = m_extent.height,
        .format = ResourceFormat::RGBA8_SRGB,
    };
    m_scene_color = graph.addResource("scene_color", scene_color_desc);
    ResourceDesc scene_depth_desc{
        .width = m_extent.width,
        .height = m_extent.height,
        .format = ResourceFormat::D32F,
    };
    m_scene_depth = graph.addResource("scene_depth", scene_depth_desc);
  }
  void onResolve(RenderGraph &graph) override {
    // у этого прохода нету зависимостей
  }

  void onBuild(RenderGraph &graph) override {
    graph.addPass("OpaqueScene")
        .write(m_scene_color)
        .write(m_scene_depth)
        .setExecuteCallback([this](CommandList &cmd) {
          if (m_draw)
            m_draw(cmd);
        });
  }

private:
  ResourceView m_scene_color, m_scene_depth;
};

class ImGuiSlot : public RenderSlot {
public:
  std::string_view name() const override { return "IMGUI"; }

  void onSetup(RenderGraph &graph) override {
    m_scene_color = graph.getResource("scene_color");
  }
  void onResolve(RenderGraph &graph) override {
    // у этого прохода нету зависимостей
  }

  void onBuild(RenderGraph &graph) override {
    graph.addPass("ImGui")
        .write(m_scene_color)
        .setExecuteCallback([this](CommandList &cmd) {
          if (m_draw)
            m_draw(cmd);
        });
  }

private:
  ResourceView m_scene_color;
};

} // namespace ssme
