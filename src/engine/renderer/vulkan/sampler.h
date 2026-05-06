#pragma once

#include <vulkan/vulkan.hpp>

namespace ck::vulkan {

class Context;

// RAII over vk::Sampler. Knobs are minimal for now; will grow when
// Renderer2D / Material need anisotropy / compare ops / explicit LOD.
class Sampler {
public:
  struct CreateInfo {
    vk::Filter mag_filter = vk::Filter::eLinear;
    vk::Filter min_filter = vk::Filter::eLinear;
    vk::SamplerAddressMode address_mode = vk::SamplerAddressMode::eRepeat;
    vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eLinear;
  };

  // Convenience: linear filter + repeat address, default mipmap mode.
  explicit Sampler(Context& ctx);
  Sampler(Context& ctx, const CreateInfo& info);
  ~Sampler();

  Sampler(const Sampler&) = delete;
  Sampler& operator=(const Sampler&) = delete;
  Sampler(Sampler&&) = delete;
  Sampler& operator=(Sampler&&) = delete;

  vk::Sampler handle() const { return sampler_; }

private:
  vk::Device device_;
  vk::Sampler sampler_;
};

}  // namespace ck::vulkan