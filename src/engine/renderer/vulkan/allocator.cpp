#include "renderer/vulkan/allocator.h"

#include <volk.h>

#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/vulkan/context.h"

namespace ck::vulkan {

Allocator::Allocator(Context& ctx) {
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
    CK_ENGINE_FATAL("vmaCreateAllocator failed");
    return;
  }

  CK_ENGINE_INFO("VMA allocator ready");
}

Allocator::~Allocator() {
  CK_PROFILE_FUNCTION();
  if (allocator_) vmaDestroyAllocator(allocator_);
}

}  // namespace ck::vulkan