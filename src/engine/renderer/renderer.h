#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "core/core.h"
#include "renderer/vulkan/frame.h"  // for kFramesInFlight
#include "renderer/vulkan/uniform_buffer.h"

namespace ck {
class Window;
}

namespace ck::vulkan {
class Context;
class Swapchain;
class Allocator;
class Buffer;
class Image;
class Sampler;
class DescriptorPool;
class DescriptorSetLayout;
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
  struct CameraData {
    glm::mat4 view_proj;
  };

  void RecreateSwapchain();

  Window& window_;
  Scope<vulkan::Context> context_;
  Scope<vulkan::Allocator> allocator_;
  Scope<vulkan::Swapchain> swapchain_;
  std::array<Scope<vulkan::Frame>, vulkan::kFramesInFlight> frames_;
  std::vector<vk::Semaphore> render_finished_;  // per-image
  uint32_t current_frame_ = 0;
  uint32_t image_index_ = 0;
  bool frame_active_ = false;
  bool resize_pending_ = false;
  std::chrono::steady_clock::time_point start_time_;

  Scope<vulkan::SlangCompiler> slang_;
  Scope<vulkan::Image> texture_;
  Scope<vulkan::Sampler> sampler_;
  Scope<vulkan::DescriptorPool> descriptor_pool_;
  Scope<vulkan::DescriptorSetLayout> descriptor_set_layout_;
  std::array<vk::DescriptorSet, vulkan::kFramesInFlight> descriptor_sets_;
  Scope<vulkan::UniformBuffer<CameraData>> camera_ubo_;
  Scope<vulkan::ShaderModule> quad_shader_;
  Scope<vulkan::GraphicsPipeline> quad_pipeline_;
  Scope<vulkan::Buffer> quad_vbo_;
  Scope<vulkan::Buffer> quad_ibo_;
};

}  // namespace ck