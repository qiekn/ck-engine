#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/core.h"
#include "renderer/vulkan/frame.h"  // for kFramesInFlight

namespace ck {
class Window;
}

namespace ck::vulkan {
class Context;
class Swapchain;
class SlangCompiler;
class ShaderModule;
class GraphicsPipeline;
}

namespace ck {

class Renderer {
public:
  explicit Renderer(Window& window);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  void BeginFrame();
  void EndFrame();
  void OnResize(uint32_t width, uint32_t height);

private:
  void RecreateSwapchain();

  Window& window_;
  Scope<vulkan::Context> context_;
  Scope<vulkan::Swapchain> swapchain_;
  std::array<Scope<vulkan::Frame>, vulkan::kFramesInFlight> frames_;
  std::vector<vk::Semaphore> render_finished_;  // per-image
  uint32_t current_frame_ = 0;
  uint32_t image_index_ = 0;
  bool frame_active_ = false;
  bool resize_pending_ = false;
  std::chrono::steady_clock::time_point start_time_;

  Scope<vulkan::SlangCompiler> slang_;
  Scope<vulkan::ShaderModule> triangle_shader_;
  Scope<vulkan::GraphicsPipeline> triangle_pipeline_;
};

}  // namespace ck