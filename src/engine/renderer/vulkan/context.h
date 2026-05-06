#pragma once

#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck {
class Window;
}

namespace ck::vulkan {

// Owns the Vulkan instance / debug messenger / surface / device / queue.
// Sibling classes (Swapchain, Frame, ...) read handles via getters.
class Context {
public:
  explicit Context(Window& window);
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(Context&&) = delete;

  vk::Instance       instance()        const { return instance_; }
  vk::PhysicalDevice physical_device() const { return physical_device_; }
  vk::Device         device()          const { return device_; }
  vk::SurfaceKHR     surface()         const { return surface_; }
  uint32_t           graphics_family() const { return graphics_family_; }
  vk::Queue          graphics_queue()  const { return graphics_queue_; }

private:
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT debug_messenger_;
  vk::SurfaceKHR surface_;
  vk::PhysicalDevice physical_device_;
  vk::Device device_;
  uint32_t graphics_family_ = ~0u;
  vk::Queue graphics_queue_;
};

}  // namespace ck::vulkan