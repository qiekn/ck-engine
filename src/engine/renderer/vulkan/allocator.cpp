#include "renderer/vulkan/allocator.h"

#include <volk.h>

#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/vulkan/context.h"

namespace ck::vulkan {

Allocator::Allocator(Context& ctx) : ctx_(&ctx) {
  CK_PROFILE_FUNCTION();

  // VMA_DYNAMIC_VULKAN_FUNCTIONS is on (vma.cpp); we hand VMA the two
  // proc-addr getters that volk has loaded, and VMA pulls the rest itself.
  VmaVulkanFunctions fns{};
  fns.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  fns.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo info{};
  info.physicalDevice = static_cast<VkPhysicalDevice>(ctx.physical_device());
  info.device = static_cast<VkDevice>(ctx.device());
  info.instance = static_cast<VkInstance>(ctx.instance());
  info.vulkanApiVersion = VK_API_VERSION_1_3;
  info.pVulkanFunctions = &fns;

  if (vmaCreateAllocator(&info, &allocator_) != VK_SUCCESS) {
    ck::log::fatal("vmaCreateAllocator failed");
    return;
  }

  // Transient command pool + fence used by ImmediateSubmit for one-shot
  // staging copies and layout transitions.
  vk::CommandPoolCreateInfo pci{};
  pci.flags = vk::CommandPoolCreateFlagBits::eTransient;
  pci.queueFamilyIndex = ctx.graphics_family();
  transient_pool_ = ctx.device().createCommandPool(pci);

  vk::FenceCreateInfo fci{};
  fence_ = ctx.device().createFence(fci);

  ck::log::info("VMA allocator ready");
}

Allocator::~Allocator() {
  CK_PROFILE_FUNCTION();
  if (ctx_ && ctx_->device()) {
    if (fence_) ctx_->device().destroyFence(fence_);
    if (transient_pool_) ctx_->device().destroyCommandPool(transient_pool_);
  }
  if (allocator_) vmaDestroyAllocator(allocator_);
}

void Allocator::ImmediateSubmit(std::function<void(vk::CommandBuffer)> fn) {
  CK_PROFILE_FUNCTION();
  vk::Device dev = ctx_->device();

  vk::CommandBufferAllocateInfo ai{};
  ai.commandPool = transient_pool_;
  ai.level = vk::CommandBufferLevel::ePrimary;
  ai.commandBufferCount = 1;
  vk::CommandBuffer cmd = dev.allocateCommandBuffers(ai)[0];

  vk::CommandBufferBeginInfo bi{};
  bi.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  cmd.begin(bi);
  fn(cmd);
  cmd.end();

  vk::CommandBufferSubmitInfo csi{};
  csi.commandBuffer = cmd;
  vk::SubmitInfo2 si{};
  si.commandBufferInfoCount = 1;
  si.pCommandBufferInfos = &csi;

  ctx_->graphics_queue().submit2(si, fence_);
  (void)dev.waitForFences(fence_, VK_TRUE, UINT64_MAX);
  dev.resetFences(fence_);
  dev.freeCommandBuffers(transient_pool_, cmd);
}

}  // namespace ck::vulkan