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

// Hello-triangle pipeline: dynamic rendering, no descriptor sets, no push
// constants, dynamic viewport+scissor. Both vertex and fragment entry
// points must be named "main" inside |shader|. |color_format| has to
// match the format declared at vk::CommandBuffer::beginRendering time
// (i.e., the swapchain format). Vertex input bindings/attributes come
// from |vertex_input|; pass empty spans for SV_VertexID-driven shaders
// (no longer used after phase 5.1.3).
class GraphicsPipeline {
public:
  GraphicsPipeline(Context& ctx, const ShaderModule& shader, vk::Format color_format,
                   const VertexInput& vertex_input);
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