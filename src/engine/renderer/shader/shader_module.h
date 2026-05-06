#pragma once

#include <cstdint>
#include <span>
#include <vulkan/vulkan.hpp>

namespace ck::vulkan {

class Context;

// RAII over vk::ShaderModule. A single Slang module can hold multiple entry
// points (vertex + fragment, ...); pipelines pick which entry point to use via
// (stage, pName) when configuring shader stages.
class ShaderModule {
public:
  ShaderModule(Context& ctx, std::span<const uint32_t> spirv);
  ~ShaderModule();

  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ShaderModule(ShaderModule&&) = delete;
  ShaderModule& operator=(ShaderModule&&) = delete;

  vk::ShaderModule handle() const { return module_; }

private:
  vk::Device device_;
  vk::ShaderModule module_;
};

}  // namespace ck::vulkan