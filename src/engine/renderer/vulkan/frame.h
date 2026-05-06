#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck::vulkan {

inline constexpr uint32_t kFramesInFlight = 2;

class Context;

// Per-frame-in-flight resources:
//   - dedicated command pool + primary command buffer
//   - image_available  : signalled by acquire, waited on by submit
//   - in_flight        : CPU-GPU sync; signalled at submit, waited on next BeginFrame
//                        (created in signalled state so first BeginFrame returns immediately)
//
// render_finished must be per-swapchain-image (not per-frame-in-flight) per Vulkan binary
// semaphore reuse rules; it lives in Renderer.
class Frame {
public:
  explicit Frame(Context& ctx);
  ~Frame();

  Frame(const Frame&) = delete;
  Frame& operator=(const Frame&) = delete;
  Frame(Frame&&) = delete;
  Frame& operator=(Frame&&) = delete;

  vk::CommandPool   command_pool()    const { return command_pool_; }
  vk::CommandBuffer command_buffer()  const { return command_buffer_; }
  vk::Semaphore     image_available() const { return image_available_; }
  vk::Fence         in_flight()       const { return in_flight_; }

private:
  Context& ctx_;
  vk::CommandPool command_pool_;
  vk::CommandBuffer command_buffer_;
  vk::Semaphore image_available_;
  vk::Fence in_flight_;
};

}  // namespace ck::vulkan