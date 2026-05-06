#include "renderer/vulkan/sampler.h"

#include "renderer/vulkan/context.h"

namespace ck::vulkan {

Sampler::Sampler(Context& ctx) : Sampler(ctx, CreateInfo{}) {}

Sampler::Sampler(Context& ctx, const CreateInfo& info) : device_(ctx.device()) {
  vk::SamplerCreateInfo sci{};
  sci.magFilter = info.mag_filter;
  sci.minFilter = info.min_filter;
  sci.mipmapMode = info.mipmap_mode;
  sci.addressModeU = info.address_mode;
  sci.addressModeV = info.address_mode;
  sci.addressModeW = info.address_mode;
  sci.minLod = 0.0f;
  sci.maxLod = VK_LOD_CLAMP_NONE;
  sci.maxAnisotropy = 1.0f;
  sampler_ = device_.createSampler(sci);
}

Sampler::~Sampler() {
  if (device_ && sampler_) device_.destroySampler(sampler_);
}

}  // namespace ck::vulkan