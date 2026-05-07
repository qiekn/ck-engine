#include "material.h"

#include <span>

#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/shader/graphics_pipeline.h"
#include "renderer/shader/shader_module.h"
#include "renderer/shader/slang_compiler.h"
#include "renderer/vulkan/context.h"
#include "renderer/vulkan/descriptor.h"
#include "renderer/vulkan/image.h"
#include "renderer/vulkan/sampler.h"

namespace ck {

Material::Material(vulkan::Context& ctx, vulkan::SlangCompiler& compiler,
                   vulkan::DescriptorPool& pool, const Spec& spec)
    : ctx_(&ctx) {
  CK_PROFILE_FUNCTION();

  auto spirv = compiler.CompileToSpirv(spec.shader_path);
  CK_ASSERT(!spirv.empty(),
                   "Slang compile failed for material shader");
  shader_ = CreateScope<vulkan::ShaderModule>(ctx, std::span{spirv});

  set_layout_ = CreateScope<vulkan::DescriptorSetLayout>(
      ctx, std::span{spec.bindings.data(), spec.bindings.size()});

  for (auto& set : descriptor_sets_) {
    set = pool.Allocate(set_layout_->handle());
  }

  vk::DescriptorSetLayout layout = set_layout_->handle();
  pipeline_ = CreateScope<vulkan::GraphicsPipeline>(
      ctx, *shader_, spec.color_format, spec.vertex_input,
      std::span{&layout, 1}, ctx.pipeline_cache());

  ck::log::info("Material ready: {}",
                 spec.shader_path.filename().string());
}

Material::~Material() = default;

void Material::SetUniformBuffer(uint32_t binding, uint32_t frame_index,
                                vk::Buffer buffer, vk::DeviceSize range) {
  vulkan::DescriptorWriter writer;
  writer.WriteBuffer(binding, buffer, 0, range,
                     vk::DescriptorType::eUniformBuffer);
  writer.Update(*ctx_, descriptor_sets_[frame_index]);
}

void Material::SetTexture(uint32_t binding, const vulkan::Image& image,
                          const vulkan::Sampler& sampler) {
  for (auto set : descriptor_sets_) {
    vulkan::DescriptorWriter writer;
    writer.WriteImage(binding, image.view(), sampler.handle(),
                      vk::ImageLayout::eShaderReadOnlyOptimal,
                      vk::DescriptorType::eCombinedImageSampler);
    writer.Update(*ctx_, set);
  }
}

vk::Pipeline       Material::pipeline()        const { return pipeline_->handle(); }
vk::PipelineLayout Material::pipeline_layout() const { return pipeline_->layout(); }

}  // namespace ck
