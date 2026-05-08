#pragma once

#include <span>
#include <vulkan/vulkan.hpp>

namespace ck::vulkan {

class Context;
class ShaderModule;

// Vertex-input description handed to GraphicsPipeline. Caller owns the
// underlying arrays and must keep them alive through the constructor call;
// the ctor copies their contents into vk::PipelineVertexInputStateCreateInfo.
struct VertexInput {
  std::span<const vk::VertexInputBindingDescription> bindings;
  std::span<const vk::VertexInputAttributeDescription> attributes;
};

// Pipeline configuration. Fields with sensible defaults can be left alone
// for the common 2D quad case; 3D mesh callers opt into depth state +
// back-face culling. depth_format = Undefined disables the depth attachment
// slot in PipelineRenderingCreateInfo (still valid even when depth_test is
// false, e.g. for a 2D-only pass that won't run alongside 3D draws).
struct GraphicsPipelineSpec {
  vk::Format color_format = vk::Format::eUndefined;
  vk::Format depth_format = vk::Format::eUndefined;
  bool depth_test_enable = false;
  bool depth_write_enable = false;
  vk::CompareOp depth_compare_op = vk::CompareOp::eLess;
  bool blend_enable = false;
  vk::CullModeFlagBits cull_mode = vk::CullModeFlagBits::eNone;
  vk::FrontFace front_face = vk::FrontFace::eCounterClockwise;
  std::span<const vk::DescriptorSetLayout> set_layouts = {};
  std::span<const vk::PushConstantRange> push_constants = {};
};

// Dynamic-rendering graphics pipeline. Both vertex and fragment entry
// points must be named "main" inside |shader|. |spec.color_format| has to
// match the format declared at vk::CommandBuffer::beginRendering time. If
// |spec.depth_format| is set (non-Undefined), the matching beginRendering
// call must also bind a depth attachment of that format.
class GraphicsPipeline {
public:
  GraphicsPipeline(Context& ctx, const ShaderModule& shader,
                   const VertexInput& vertex_input,
                   const GraphicsPipelineSpec& spec,
                   vk::PipelineCache cache = {});
  ~GraphicsPipeline();

  GraphicsPipeline(const GraphicsPipeline&) = delete;
  GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
  GraphicsPipeline(GraphicsPipeline&&) = delete;
  GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

  vk::Pipeline handle() const { return pipeline_; }
  vk::PipelineLayout layout() const { return layout_; }

private:
  vk::Device device_;
  vk::PipelineLayout layout_;
  vk::Pipeline pipeline_;
};

}  // namespace ck::vulkan
