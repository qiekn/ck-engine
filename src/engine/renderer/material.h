#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/core.h"
#include "renderer/shader/graphics_pipeline.h"  // for VertexInput
#include "renderer/vulkan/frame.h"              // for kFramesInFlight

namespace ck::vulkan {
class Context;
class SlangCompiler;
class DescriptorPool;
class DescriptorSetLayout;
class ShaderModule;
class Image;
class Sampler;
}  // namespace ck::vulkan

namespace ck {

// User-facing render unit. Combines:
//   - a Slang shader (compiled at construction time)
//   - a descriptor set layout described by Spec.bindings
//   - one descriptor set per frame-in-flight, so per-frame UBO updates
//     don't race the GPU
//   - a graphics pipeline built against the engine-wide PipelineCache
//
// 5.3.2: the class lands and is constructed alongside Renderer's
// existing hand-rolled path. 5.3.3 will replace that path with this.
class Material {
public:
  struct Spec {
    std::filesystem::path shader_path;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::Format color_format;
    vulkan::VertexInput vertex_input;  // spans must outlive the ctor call
  };

  Material(vulkan::Context& ctx, vulkan::SlangCompiler& compiler,
           vulkan::DescriptorPool& pool, const Spec& spec);
  ~Material();

  Material(const Material&) = delete;
  Material& operator=(const Material&) = delete;
  Material(Material&&) = delete;
  Material& operator=(Material&&) = delete;

  // Update one frame slot's descriptor for a uniform-buffer binding.
  // Caller must serialize against GPU use of that frame's set (i.e.,
  // call only after the frame-in-flight fence wait for |frame_index|).
  void SetUniformBuffer(uint32_t binding, uint32_t frame_index,
                        vk::Buffer buffer, vk::DeviceSize range);

  // Write the same image+sampler binding into every frame slot. Safe
  // for read-only textures owned for the material's lifetime.
  void SetTexture(uint32_t binding, const vulkan::Image& image,
                  const vulkan::Sampler& sampler);

  vk::Pipeline       pipeline()                          const;
  vk::PipelineLayout pipeline_layout()                   const;
  vk::DescriptorSet  descriptor_set(uint32_t frame_index) const {
    return descriptor_sets_[frame_index];
  }

private:
  vulkan::Context* ctx_;
  Scope<vulkan::ShaderModule> shader_;
  Scope<vulkan::DescriptorSetLayout> set_layout_;
  std::array<vk::DescriptorSet, vulkan::kFramesInFlight> descriptor_sets_{};
  Scope<vulkan::GraphicsPipeline> pipeline_;
};

}  // namespace ck
