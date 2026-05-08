#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "core/core.h"
#include "renderer/camera.h"
#include "renderer/vulkan/frame.h"  // for kFramesInFlight

namespace ck {
class Window;
}

namespace ck::vulkan {
class Context;
class Swapchain;
class Allocator;
class Image;
class SlangCompiler;
}

namespace ck {

class Renderer {
public:
  // Recorded inside EndFrame's swapchain pass (loadOp=Clear). Set to {} to
  // disable the imgui pass — useful for headless smoke tests, but in normal
  // use the offscreen color_target only reaches the screen via the imgui
  // ViewportPanel that samples it.
  using ImGuiRenderCallback = std::function<void(vk::CommandBuffer)>;

  // Fired after color_target_ has just been (re)created. ImGuiLayer uses
  // this to drop the stale ImGui_ImplVulkan descriptor set and register a
  // new one against the fresh image view.
  using ColorTargetCallback = std::function<void()>;

  explicit Renderer(Window& window);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  void BeginFrame();
  void EndFrame();
  void OnResize(uint32_t width, uint32_t height);
  // Editor hands the Viewport panel's current content size in here. Stored
  // until the next BeginFrame, which drains in-flight work and rebuilds
  // color_target_ + camera at the new extent. Cheap to call every frame —
  // a no-op when the size hasn't actually changed.
  void OnViewportResize(uint32_t width, uint32_t height);

  // Per-frame view_projection used by Renderer2D / Renderer3D draws this
  // frame. Sticky: editors call it once per OnUpdate and the value persists
  // until overwritten. nullopt restores the internal orthographic fallback
  // (the stock 2D Camera, used by sandbox).
  void SetActiveCamera(const glm::mat4& view_projection);
  void ResetActiveCamera();

  void SetImGuiRenderCallback(ImGuiRenderCallback cb) { imgui_render_ = std::move(cb); }
  // Registers cb and fires it once with the current color_target so the
  // caller can stand up its initial state without a separate path.
  void SetColorTargetCallback(ColorTargetCallback cb);

  vulkan::Context&   context()      { return *context_; }
  vulkan::Allocator& allocator()    { return *allocator_; }
  vulkan::Swapchain& swapchain()    { return *swapchain_; }
  vk::ImageView      color_target_view() const;
  // Editor-facing handle: editor camera pokes position/zoom on this each
  // frame. Renderer keeps SetViewport ownership so width/height stay synced
  // to color_target_'s extent.
  Camera&            GetCamera()    { return camera_; }
  // Color target extent (the Viewport-panel-driven offscreen extent). Used
  // by editor cameras to compute aspect for their own perspective math.
  vk::Extent2D       color_target_extent() const;

private:
  void RecreateSwapchain();
  void RecreateColorTarget(vk::Extent2D extent);
  void ApplyPendingViewportResize();

  Window& window_;
  Scope<vulkan::Context> context_;
  Scope<vulkan::Allocator> allocator_;
  Scope<vulkan::Swapchain> swapchain_;
  Scope<vulkan::Image> color_target_;  // offscreen render target; sampled by imgui's ViewportPanel
  Scope<vulkan::Image> depth_target_;  // depth-only D32_SFLOAT, parallel to color_target_
  std::array<Scope<vulkan::Frame>, vulkan::kFramesInFlight> frames_;
  std::vector<vk::Semaphore> render_finished_;  // per-image
  uint32_t current_frame_ = 0;
  uint32_t image_index_ = 0;
  bool frame_active_ = false;
  bool resize_pending_ = false;
  vk::Extent2D pending_viewport_extent_{};  // {0,0} = no resize requested
  std::chrono::steady_clock::time_point start_time_;

  Camera camera_;
  // Set by layers to override camera_'s ortho fallback (editor's perspective
  // arcball pushes here). nullopt = use camera_.view_projection().
  std::optional<glm::mat4> active_view_projection_;
  Scope<vulkan::SlangCompiler> slang_;
  ImGuiRenderCallback imgui_render_;
  ColorTargetCallback color_target_changed_;
};

}  // namespace ck