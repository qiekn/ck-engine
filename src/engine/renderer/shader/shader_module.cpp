#include "renderer/shader/shader_module.h"

#include "renderer/vulkan/context.h"

namespace ck::vulkan {

ShaderModule::ShaderModule(Context& ctx, std::span<const uint32_t> spirv)
    : device_(ctx.device()) {
  vk::ShaderModuleCreateInfo info{};
  info.codeSize = spirv.size_bytes();
  info.pCode = spirv.data();
  module_ = device_.createShaderModule(info);
}

ShaderModule::~ShaderModule() {
  if (device_ && module_) device_.destroyShaderModule(module_);
}

}  // namespace ck::vulkan