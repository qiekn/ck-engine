#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck::vulkan {

inline constexpr uint32_t kFramesInFlight = 2;

class Context;

// Per-frame resources for a frame-in-flight slot:
//   - dedicated command pool + primary command buffer
//   - image_available  : signalled by acquire, waited on by submit
//   - render_finished  : signalled by submit,  waited on by present
//   - in_flight        : CPU-GPU sync; created in signalled state so the very
//                        first BeginFrame's waitForFences returns immediately
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
  vk::Semaphore     render_finished() const { return render_finished_; }
  vk::Fence         in_flight()       const { return in_flight_; }

private:
  Context& ctx_;
  vk::CommandPool command_pool_;
  vk::CommandBuffer command_buffer_;
  vk::Semaphore image_available_;
  vk::Semaphore render_finished_;
  vk::Fence in_flight_;
};

}  // namespace ck::vulkan