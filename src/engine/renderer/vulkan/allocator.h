#pragma once

#include <vk_mem_alloc.h>

namespace ck::vulkan {

class Context;

// RAII over VmaAllocator. Constructed after Context (needs instance /
// physical / device handles); destroyed before Context. Buffer / Image
// (5.1.2+) pull `handle()` to back VkBuffer / VkImage with VMA-allocated
// memory.
class Allocator {
public:
  explicit Allocator(Context& ctx);
  ~Allocator();

  Allocator(const Allocator&) = delete;
  Allocator& operator=(const Allocator&) = delete;
  Allocator(Allocator&&) = delete;
  Allocator& operator=(Allocator&&) = delete;

  VmaAllocator handle() const { return allocator_; }

private:
  VmaAllocator allocator_ = VK_NULL_HANDLE;
};

}  // namespace ck::vulkan