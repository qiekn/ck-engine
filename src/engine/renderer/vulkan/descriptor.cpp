#include "renderer/vulkan/descriptor.h"

#include "core/log.h"
#include "renderer/vulkan/context.h"

namespace ck::vulkan {

// - -----------------------------------------------------------------------------: DescriptorPool

DescriptorPool::DescriptorPool(Context& ctx, std::span<const PoolSize> sizes, uint32_t max_sets)
    : device_(ctx.device()) {
  std::vector<vk::DescriptorPoolSize> raw_sizes;
  raw_sizes.reserve(sizes.size());
  for (const auto& s : sizes) {
    vk::DescriptorPoolSize ps{};
    ps.type = s.type;
    ps.descriptorCount = s.count;
    raw_sizes.push_back(ps);
  }

  vk::DescriptorPoolCreateInfo info{};
  info.maxSets = max_sets;
  info.poolSizeCount = static_cast<uint32_t>(raw_sizes.size());
  info.pPoolSizes = raw_sizes.data();
  pool_ = device_.createDescriptorPool(info);
}

DescriptorPool::~DescriptorPool() {
  if (device_ && pool_) device_.destroyDescriptorPool(pool_);
}

vk::DescriptorSet DescriptorPool::Allocate(vk::DescriptorSetLayout layout) {
  vk::DescriptorSetAllocateInfo info{};
  info.descriptorPool = pool_;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &layout;
  return device_.allocateDescriptorSets(info)[0];
}

// - -----------------------------------------------------------------------------: DescriptorSetLayout

DescriptorSetLayout::DescriptorSetLayout(Context& ctx,
                                         std::span<const vk::DescriptorSetLayoutBinding> bindings)
    : device_(ctx.device()) {
  vk::DescriptorSetLayoutCreateInfo info{};
  info.bindingCount = static_cast<uint32_t>(bindings.size());
  info.pBindings = bindings.data();
  layout_ = device_.createDescriptorSetLayout(info);
}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (device_ && layout_) device_.destroyDescriptorSetLayout(layout_);
}

// - -----------------------------------------------------------------------------: DescriptorWriter

DescriptorWriter& DescriptorWriter::WriteBuffer(uint32_t binding, vk::Buffer buffer,
                                                vk::DeviceSize offset, vk::DeviceSize range,
                                                vk::DescriptorType type) {
  vk::DescriptorBufferInfo bi{};
  bi.buffer = buffer;
  bi.offset = offset;
  bi.range = range;
  buffer_infos_.push_back(bi);

  vk::WriteDescriptorSet w{};
  w.dstBinding = binding;
  w.dstArrayElement = 0;
  w.descriptorCount = 1;
  w.descriptorType = type;
  w.pBufferInfo = &buffer_infos_.back();
  writes_.push_back(w);
  return *this;
}

DescriptorWriter& DescriptorWriter::WriteImage(uint32_t binding, vk::ImageView view,
                                               vk::Sampler sampler, vk::ImageLayout layout,
                                               vk::DescriptorType type) {
  vk::DescriptorImageInfo ii{};
  ii.sampler = sampler;
  ii.imageView = view;
  ii.imageLayout = layout;
  image_infos_.push_back(ii);

  vk::WriteDescriptorSet w{};
  w.dstBinding = binding;
  w.dstArrayElement = 0;
  w.descriptorCount = 1;
  w.descriptorType = type;
  w.pImageInfo = &image_infos_.back();
  writes_.push_back(w);
  return *this;
}

void DescriptorWriter::Update(Context& ctx, vk::DescriptorSet set) {
  for (auto& w : writes_) w.dstSet = set;
  ctx.device().updateDescriptorSets(writes_, {});
  buffer_infos_.clear();
  image_infos_.clear();
  writes_.clear();
}

}  // namespace ck::vulkan