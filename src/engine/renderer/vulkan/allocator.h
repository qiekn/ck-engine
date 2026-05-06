#pragma once

#include <functional>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace ck::vulkan {

class Context;

// RAII over VmaAllocator + a transient command pool / fence for one-shot
// uploads. Constructed after Context (needs instance / physical / device);
// destroyed before Context.
class Allocator {
public:
  explicit Allocator(Context& ctx);
  ~Allocator();

  Allocator(const Allocator&) = delete;
  Allocator& operator=(const Allocator&) = delete;
  Allocator(Allocator&&) = delete;
  Allocator& operator=(Allocator&&) = delete;

  VmaAllocator handle() const { return allocator_; }

  // One-shot command recording: allocate a primary command buffer from the
  // transient pool, run |fn| between begin/end, submit to the graphics
  // queue, and wait on a fence before returning. Blocking — use only
  // outside the per-frame hot path (e.g. asset upload at startup).
  void ImmediateSubmit(std::function<void(vk::CommandBuffer)> fn);

private:
  Context* ctx_ = nullptr;
  VmaAllocator allocator_ = VK_NULL_HANDLE;
  vk::CommandPool transient_pool_;
  vk::Fence fence_;
};

}  // namespace ck::vulkan