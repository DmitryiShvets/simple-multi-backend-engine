// godrays_slot.h
#pragma once
#include "core/render_types.h"
#include "core/rid.h"
#include "graph/render_graph.h"
#include "graph/render_slot.h"
#include "resource_handle.h"
#include "resources/texture.h"

namespace ssme {

class SinkSlot : public RenderSlot {
public:
  std::string_view name() const override { return "SINK"; }

  void setSwapchain(RID color, RID depth, Extent2D extent) {
    m_color_rid = color;
    m_depth_rid = depth;
    m_extent = extent;
  }

  void onSetup(RenderGraph &graph) override {
    ResourceDesc swapchain_color_desc{
        .width = 600,
        .height = 400,
        .format = ResourceFormat::RGBA16F,
    };
    m_swapchain_color = graph.importResource(
        "swapchain/color", swapchain_color_desc, ResourceState::PRESENT);
    ResourceDesc swapchain_depth_desc{
        .width = 600,
        .height = 400,
        .format = ResourceFormat::D32F,
    };
    m_swapchain_depth =
        graph.importResource("swapchain/depth", swapchain_depth_desc,
                             ResourceState::DEPTH_ATTACHMENT);
    graph.bindImport(m_swapchain_color, m_color_rid);
    graph.bindImport(m_swapchain_depth, m_depth_rid);
  }
  void onResolve(RenderGraph &graph) override {
    m_final_color = graph.getResource("scene_color");
  }

  void onBuild(RenderGraph &graph) override {
    graph.addPass("Present")
        .read(m_final_color)
        .write(m_swapchain_color)
        .write(m_swapchain_depth)
        .setLoadOp(LoadOp::LOAD)
        .setExecuteCallback([](auto &cmd) {});
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
        .width = 600,
        .height = 400,
        .format = ResourceFormat::RGBA16F,
    };
    m_scene_color = graph.addResource("scene_color", scene_color_desc);
    ResourceDesc scene_depth_desc{
        .width = 600,
        .height = 400,
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
        .setExecuteCallback([](auto &cmd) {});
  }

private:
  ResourceView m_scene_color, m_scene_depth;
};

class ImGuiSlot : public RenderSlot {
public:
  std::string_view name() const override { return "IMGUI"; }

  void onSetup(RenderGraph &graph) override {
    ResourceDesc scene_color_desc{
        .width = 600,
        .height = 400,
        .format = ResourceFormat::RGBA16F,
    };
    m_scene_color = graph.addResource("scene_color", scene_color_desc);
    ResourceDesc scene_depth_desc{
        .width = 600,
        .height = 400,
        .format = ResourceFormat::D32F,
    };
    m_scene_depth = graph.addResource("scene_depth", scene_depth_desc);
  }
  void onResolve(RenderGraph &graph) override {
    // у этого прохода нету зависимостей
  }

  void onBuild(RenderGraph &graph) override {
    graph.addPass("ImGui")
        .write(m_scene_color)
        .write(m_scene_depth)
        .setExecuteCallback([](auto &cmd) {});
  }

private:
  ResourceView m_scene_color, m_scene_depth;
};

} // namespace ssme
