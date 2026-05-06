#pragma once

#include <cstdint>
#include <deque>
#include <span>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace ck::vulkan {

class Context;

// RAII over vk::DescriptorPool. Sets allocated from this pool are freed
// implicitly when the pool is destroyed.
class DescriptorPool {
public:
  struct PoolSize {
    vk::DescriptorType type;
    uint32_t count;
  };

  DescriptorPool(Context& ctx, std::span<const PoolSize> sizes, uint32_t max_sets);
  ~DescriptorPool();

  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool& operator=(const DescriptorPool&) = delete;
  DescriptorPool(DescriptorPool&&) = delete;
  DescriptorPool& operator=(DescriptorPool&&) = delete;

  vk::DescriptorPool handle() const { return pool_; }

  vk::DescriptorSet Allocate(vk::DescriptorSetLayout layout);

private:
  vk::Device device_;
  vk::DescriptorPool pool_;
};

// RAII over vk::DescriptorSetLayout. Caller fills the binding list
// imperatively (binding index, descriptor type, stage flags, descriptor
// count) and constructs in one shot.
class DescriptorSetLayout {
public:
  DescriptorSetLayout(Context& ctx, std::span<const vk::DescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayout();

  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout(DescriptorSetLayout&&) = delete;
  DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

  vk::DescriptorSetLayout handle() const { return layout_; }

private:
  vk::Device device_;
  vk::DescriptorSetLayout layout_;
};

// Chained collector for vkUpdateDescriptorSets. Buffer/image infos live
// inside std::deque so their addresses remain stable across pushes; the
// writes_ vector then holds vk::WriteDescriptorSet entries pointing at
// those stable addresses.
class DescriptorWriter {
public:
  DescriptorWriter& WriteBuffer(uint32_t binding, vk::Buffer buffer,
                                vk::DeviceSize offset, vk::DeviceSize range,
                                vk::DescriptorType type);
  DescriptorWriter& WriteImage(uint32_t binding, vk::ImageView view,
                               vk::Sampler sampler, vk::ImageLayout layout,
                               vk::DescriptorType type);

  // Apply collected writes to |set|; clears internal state so the writer
  // can be reused.
  void Update(Context& ctx, vk::DescriptorSet set);

private:
  std::deque<vk::DescriptorBufferInfo> buffer_infos_;
  std::deque<vk::DescriptorImageInfo> image_infos_;
  std::vector<vk::WriteDescriptorSet> writes_;
};

}  // namespace ck::vulkan