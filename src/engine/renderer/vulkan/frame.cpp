#include "frame.h"

#include "context.h"

#include <volk.h>

#include "core/log.h"
#include "debug/profiler.h"

namespace ck::vulkan {

Frame::Frame(Context& ctx) : ctx_(ctx) {
  CK_PROFILE_FUNCTION();
  vk::Device dev = ctx_.device();

  vk::CommandPoolCreateInfo pool_ci{};
  pool_ci.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  pool_ci.queueFamilyIndex = ctx_.graphics_family();
  command_pool_ = dev.createCommandPool(pool_ci);

  vk::CommandBufferAllocateInfo alloc{};
  alloc.commandPool = command_pool_;
  alloc.level = vk::CommandBufferLevel::ePrimary;
  alloc.commandBufferCount = 1;
  command_buffer_ = dev.allocateCommandBuffers(alloc).front();

  vk::SemaphoreCreateInfo sem_ci{};
  image_available_ = dev.createSemaphore(sem_ci);

  vk::FenceCreateInfo fence_ci{};
  fence_ci.flags = vk::FenceCreateFlagBits::eSignaled;
  in_flight_ = dev.createFence(fence_ci);
}

Frame::~Frame() {
  CK_PROFILE_FUNCTION();
  vk::Device dev = ctx_.device();
  if (in_flight_) dev.destroyFence(in_flight_);
  if (image_available_) dev.destroySemaphore(image_available_);
  if (command_pool_) dev.destroyCommandPool(command_pool_);
}

}  // namespace ck::vulkan