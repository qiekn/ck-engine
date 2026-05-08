#include "renderer/shader/graphics_pipeline.h"

#include <array>
#include <cstdint>

#include "renderer/shader/shader_module.h"
#include "renderer/vulkan/context.h"

namespace ck::vulkan {

GraphicsPipeline::GraphicsPipeline(Context& ctx,
                                   const ShaderModule& shader,
                                   const VertexInput& vertex_input,
                                   const GraphicsPipelineSpec& spec,
                                   vk::PipelineCache cache)
    : device_(ctx.device()) {
  vk::PipelineLayoutCreateInfo layout_info{};
  layout_info.setLayoutCount = static_cast<uint32_t>(spec.set_layouts.size());
  layout_info.pSetLayouts = spec.set_layouts.data();
  layout_info.pushConstantRangeCount = static_cast<uint32_t>(spec.push_constants.size());
  layout_info.pPushConstantRanges = spec.push_constants.data();
  layout_ = device_.createPipelineLayout(layout_info);

  std::array<vk::PipelineShaderStageCreateInfo, 2> stages{};
  stages[0].stage = vk::ShaderStageFlagBits::eVertex;
  stages[0].module = shader.handle();
  stages[0].pName = "main";
  stages[1].stage = vk::ShaderStageFlagBits::eFragment;
  stages[1].module = shader.handle();
  stages[1].pName = "main";

  vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
  vertex_input_state.vertexBindingDescriptionCount =
      static_cast<uint32_t>(vertex_input.bindings.size());
  vertex_input_state.pVertexBindingDescriptions = vertex_input.bindings.data();
  vertex_input_state.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertex_input.attributes.size());
  vertex_input_state.pVertexAttributeDescriptions = vertex_input.attributes.data();

  vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.topology = vk::PrimitiveTopology::eTriangleList;

  vk::PipelineViewportStateCreateInfo viewport_state{};
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  vk::PipelineRasterizationStateCreateInfo rasterization{};
  rasterization.polygonMode = vk::PolygonMode::eFill;
  rasterization.cullMode = spec.cull_mode;
  rasterization.frontFace = spec.front_face;
  rasterization.lineWidth = 1.0f;

  vk::PipelineMultisampleStateCreateInfo multisample{};
  multisample.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
  depth_stencil.depthTestEnable = spec.depth_test_enable ? VK_TRUE : VK_FALSE;
  depth_stencil.depthWriteEnable = spec.depth_write_enable ? VK_TRUE : VK_FALSE;
  depth_stencil.depthCompareOp = spec.depth_compare_op;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;

  vk::PipelineColorBlendAttachmentState blend_attachment{};
  blend_attachment.blendEnable = spec.blend_enable ? VK_TRUE : VK_FALSE;
  if (spec.blend_enable) {
    blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
    blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;
  }
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
  rendering_info.pColorAttachmentFormats = &spec.color_format;
  rendering_info.depthAttachmentFormat = spec.depth_format;

  vk::GraphicsPipelineCreateInfo info{};
  info.pNext = &rendering_info;
  info.stageCount = static_cast<uint32_t>(stages.size());
  info.pStages = stages.data();
  info.pVertexInputState = &vertex_input_state;
  info.pInputAssemblyState = &input_assembly;
  info.pViewportState = &viewport_state;
  info.pRasterizationState = &rasterization;
  info.pMultisampleState = &multisample;
  info.pDepthStencilState = &depth_stencil;
  info.pColorBlendState = &color_blend;
  info.pDynamicState = &dynamic_state;
  info.layout = layout_;

  auto rv = device_.createGraphicsPipeline(cache, info);
  pipeline_ = rv.value;
}

GraphicsPipeline::~GraphicsPipeline() {
  if (device_) {
    if (pipeline_) device_.destroyPipeline(pipeline_);
    if (layout_) device_.destroyPipelineLayout(layout_);
  }
}

}  // namespace ck::vulkan
