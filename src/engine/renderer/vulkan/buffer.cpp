#include "renderer/vulkan/buffer.h"

#include <cstring>

#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/vulkan/allocator.h"

namespace ck::vulkan {

namespace {

VmaAllocationCreateInfo MakeAllocCreateInfo(BufferType type) {
  VmaAllocationCreateInfo aci{};
  aci.usage = VMA_MEMORY_USAGE_AUTO;
  if (type == BufferType::HostVisible) {
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;
  }
  return aci;
}

}  // namespace

Buffer::Buffer(Allocator& alloc, vk::DeviceSize size, vk::BufferUsageFlags usage, BufferType type)
    : vma_(alloc.handle()), size_(size) {
  VkBufferCreateInfo bci{};
  bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bci.size = size;
  bci.usage = static_cast<VkBufferUsageFlags>(usage);
  bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo aci = MakeAllocCreateInfo(type);
  VmaAllocationInfo info{};
  VkBuffer raw{};
  if (vmaCreateBuffer(vma_, &bci, &aci, &raw, &alloc_, &info) != VK_SUCCESS) {
    CK_ENGINE_FATAL("vmaCreateBuffer failed (size={}, usage=0x{:x})",
                    static_cast<uint64_t>(size),
                    static_cast<uint32_t>(usage));
    return;
  }
  buffer_ = raw;
  mapped_ = info.pMappedData;  // null when not host-visible
}

Buffer::~Buffer() {
  if (vma_ && buffer_) {
    vmaDestroyBuffer(vma_, static_cast<VkBuffer>(buffer_), alloc_);
  }
}

Scope<Buffer> Buffer::CreateDeviceLocal(Allocator& alloc, const void* src_data,
                                        vk::DeviceSize size, vk::BufferUsageFlags usage) {
  CK_PROFILE_FUNCTION();

  Buffer staging(alloc, size, vk::BufferUsageFlagBits::eTransferSrc, BufferType::HostVisible);
  std::memcpy(staging.mapped(), src_data, size);

  auto dst = CreateScope<Buffer>(alloc, size,
                                 usage | vk::BufferUsageFlagBits::eTransferDst,
                                 BufferType::DeviceLocal);

  alloc.ImmediateSubmit([&](vk::CommandBuffer cmd) {
    vk::BufferCopy region{};
    region.size = size;
    cmd.copyBuffer(staging.handle(), dst->handle(), region);
  });

  return dst;
}

}  // namespace ck::vulkan