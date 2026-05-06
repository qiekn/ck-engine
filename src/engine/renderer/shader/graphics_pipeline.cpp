#include "renderer/shader/graphics_pipeline.h"

#include <array>
#include <cstdint>

#include "renderer/shader/shader_module.h"
#include "renderer/vulkan/context.h"

namespace ck::vulkan {

GraphicsPipeline::GraphicsPipeline(Context& ctx,
                                   const ShaderModule& shader,
                                   vk::Format color_format)
    : device_(ctx.device()) {
  vk::PipelineLayoutCreateInfo layout_info{};
  layout_ = device_.createPipelineLayout(layout_info);

  std::array<vk::PipelineShaderStageCreateInfo, 2> stages{};
  stages[0].stage = vk::ShaderStageFlagBits::eVertex;
  stages[0].module = shader.handle();
  stages[0].pName = "main";
  stages[1].stage = vk::ShaderStageFlagBits::eFragment;
  stages[1].module = shader.handle();
  stages[1].pName = "main";

  // No vertex input: positions/colors come from SV_VertexID inside the shader.
  vk::PipelineVertexInputStateCreateInfo vertex_input{};

  vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.topology = vk::PrimitiveTopology::eTriangleList;

  vk::PipelineViewportStateCreateInfo viewport_state{};
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  vk::PipelineRasterizationStateCreateInfo rasterization{};
  rasterization.polygonMode = vk::PolygonMode::eFill;
  rasterization.cullMode = vk::CullModeFlagBits::eNone;
  rasterization.frontFace = vk::FrontFace::eCounterClockwise;
  rasterization.lineWidth = 1.0f;

  vk::PipelineMultisampleStateCreateInfo multisample{};
  multisample.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendAttachmentState blend_attachment{};
  blend_attachment.blendEnable = VK_FALSE;
  blend_attachment.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

  vk::PipelineColorBlendStateCreateInfo color_blend{};
  color_blend.attachmentCount = 1;
  color_blend.pAttachments = &blend_attachment;

  std::array<vk::DynamicState, 2> dyn_states{
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };
  vk::PipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
  dynamic_state.pDynamicStates = dyn_states.data();

  vk::PipelineRenderingCreateInfo rendering_info{};
  rendering_info.colorAttachmentCount = 1;
  rendering_info.pColorAttachmentFormats = &color_format;

  vk::GraphicsPipelineCreateInfo info{};
  info.pNext = &rendering_info;
  info.stageCount = static_cast<uint32_t>(stages.size());
  info.pStages = stages.data();
  info.pVertexInputState = &vertex_input;
  info.pInputAssemblyState = &input_assembly;
  info.pViewportState = &viewport_state;
  info.pRasterizationState = &rasterization;
  info.pMultisampleState = &multisample;
  info.pColorBlendState = &color_blend;
  info.pDynamicState = &dynamic_state;
  info.layout = layout_;

  auto rv = device_.createGraphicsPipeline({}, info);
  pipeline_ = rv.value;
}

GraphicsPipeline::~GraphicsPipeline() {
  if (device_) {
    if (pipeline_) device_.destroyPipeline(pipeline_);
    if (layout_) device_.destroyPipelineLayout(layout_);
  }
}

}  // namespace ck::vulkan