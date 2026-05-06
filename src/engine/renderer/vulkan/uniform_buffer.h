#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/core.h"
#include "renderer/vulkan/buffer.h"

namespace ck::vulkan {

class Allocator;

// Per-frame UBO ring: one HostVisible Buffer per frame in flight, all
// persistently mapped. Caller is responsible for only writing to the
// slot the GPU is no longer reading (the frame-in-flight fence wait in
// Renderer::BeginFrame guarantees this for the current_frame_ slot).
template <typename T>
class UniformBuffer {
public:
  UniformBuffer(Allocator& alloc, uint32_t frame_count) {
    buffers_.reserve(frame_count);
    for (uint32_t i = 0; i < frame_count; ++i) {
      buffers_.push_back(CreateScope<Buffer>(
          alloc, sizeof(T),
          vk::BufferUsageFlagBits::eUniformBuffer,
          BufferType::HostVisible));
    }
  }

  ~UniformBuffer() = default;

  UniformBuffer(const UniformBuffer&) = delete;
  UniformBuffer& operator=(const UniformBuffer&) = delete;
  UniformBuffer(UniformBuffer&&) = delete;
  UniformBuffer& operator=(UniformBuffer&&) = delete;

  void Write(uint32_t frame_index, const T& data) {
    std::memcpy(buffers_[frame_index]->mapped(), &data, sizeof(T));
  }

  vk::Buffer Handle(uint32_t frame_index) const {
    return buffers_[frame_index]->handle();
  }

  static constexpr vk::DeviceSize range() { return sizeof(T); }

private:
  std::vector<Scope<Buffer>> buffers_;
};

}  // namespace ck::vulkan