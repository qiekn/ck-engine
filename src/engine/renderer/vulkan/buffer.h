#pragma once

#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck::vulkan {

class Allocator;

// Memory allocation hint passed to VMA. Maps to VMA_MEMORY_USAGE_AUTO + an
// appropriate set of VmaAllocationCreateFlagBits.
enum class BufferType {
  // Host-visible memory, persistently mapped, sequential write access.
  // Used for staging buffers and per-frame UBO rings.
  HostVisible,
  // GPU-only memory, no host access. Initial data must come through a
  // staging copy (see Buffer::CreateDeviceLocal).
  DeviceLocal,
};

// RAII over a (VkBuffer, VmaAllocation) pair.
class Buffer {
public:
  Buffer(Allocator& alloc, vk::DeviceSize size, vk::BufferUsageFlags usage, BufferType type);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) = delete;
  Buffer& operator=(Buffer&&) = delete;

  vk::Buffer handle() const { return buffer_; }
  vk::DeviceSize size() const { return size_; }

  // Persistent CPU mapping; nullptr when the buffer wasn't created
  // with BufferType::HostVisible.
  void* mapped() const { return mapped_; }

  // Allocate a device-local buffer of |size| bytes with |usage| (the helper
  // ORs in eTransferDst), copy |src_data| into it via a one-shot staging
  // upload, and return ownership. Blocks on the upload fence.
  static Scope<Buffer> CreateDeviceLocal(Allocator& alloc, const void* src_data,
                                         vk::DeviceSize size, vk::BufferUsageFlags usage);

private:
  VmaAllocator vma_ = VK_NULL_HANDLE;       // borrowed; for vmaDestroyBuffer
  VmaAllocation alloc_ = VK_NULL_HANDLE;
  vk::Buffer buffer_;
  vk::DeviceSize size_ = 0;
  void* mapped_ = nullptr;
};

}  // namespace ck::vulkan