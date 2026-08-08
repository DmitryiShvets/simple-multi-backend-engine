
#pragma once
#include "core/render_types.h"
#include "graph/render_gtaph_utils.h"
#include <string_view>

namespace ssme {
// Forward-declare to avoid circular dependency
class RenderGraph;

class RenderSlot {
public:
  virtual ~RenderSlot() = default;

  // ── Identity ──
  virtual std::string_view name() const = 0;
  virtual std::string_view group() const { return {}; }

  // ── Hooks (вызываются графом) ──
  // каждый слот ДЕКЛАРИРУЕТ свои ресурсы (addResource/importResource)
  virtual void onSetup(RenderGraph &graph) {}
  //  каждый слот ПОДКЛЮЧАЕТ чужие (getResource)  ← новой фазы нет
  virtual void onResolve(RenderGraph &graph) {}
  // каждый слот объявляет пассы (addPass + read/write + callback)
  virtual void onBuild(RenderGraph &node) {}

  virtual void setExtent(Extent2D e) { m_extent = e; }
  virtual void setDrawCallback(ExecuteCallback cb) { m_draw = std::move(cb); }

protected:
  Extent2D m_extent;
  ExecuteCallback m_draw;
};
} // namespace ssme
