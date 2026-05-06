#pragma once

#include <vulkan/vulkan.hpp>

namespace ck::vulkan {

class Context;
class ShaderModule;

// Hello-triangle pipeline: dynamic rendering, no vertex input, no descriptor
// sets, no push constants, dynamic viewport+scissor. Both vertex and fragment
// entry points must be named "main" inside |shader|. |color_format| has to
// match the format declared at vk::CommandBuffer::beginRendering time
// (i.e., the swapchain format).
class GraphicsPipeline {
public:
  GraphicsPipeline(Context& ctx, const ShaderModule& shader, vk::Format color_format);
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