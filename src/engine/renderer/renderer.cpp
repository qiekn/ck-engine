#include "renderer.h"

#include "camera.h"
#include "material.h"
#include "shader/graphics_pipeline.h"
#include "shader/slang_compiler.h"
#include "vulkan/allocator.h"
#include "vulkan/buffer.h"
#include "vulkan/context.h"
#include "vulkan/descriptor.h"
#include "vulkan/image.h"
#include "vulkan/sampler.h"
#include "vulkan/swapchain.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>

#include <glm/glm.hpp>
#include <volk.h>

#include "core/log.h"
#include "core/window.h"
#include "debug/profiler.h"

namespace ck {

namespace {

void TransitionImage(vk::CommandBuffer cmd,
                     vk::Image image,
                     vk::ImageLayout old_layout,
                     vk::ImageLayout new_layout,
                     vk::PipelineStageFlags2 src_stage,
                     vk::AccessFlags2 src_access,
                     vk::PipelineStageFlags2 dst_stage,
                     vk::AccessFlags2 dst_access) {
  vk::ImageMemoryBarrier2 barrier{};
  barrier.srcStageMask = src_stage;
  barrier.srcAccessMask = src_access;
  barrier.dstStageMask = dst_stage;
  barrier.dstAccessMask = dst_access;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange = vk::ImageSubresourceRange{
      vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

  vk::DependencyInfo dep{};
  dep.imageMemoryBarrierCount = 1;
  dep.pImageMemoryBarriers = &barrier;
  cmd.pipelineBarrier2(dep);
}

}  // namespace

Renderer::Renderer(Window& window) : window_(window) {
  CK_PROFILE_FUNCTION();
  start_time_ = std::chrono::steady_clock::now();
  context_ = CreateScope<vulkan::Context>(window_);
  allocator_ = CreateScope<vulkan::Allocator>(*context_);
  swapchain_ = CreateScope<vulkan::Swapchain>(*context_, window_);
  for (auto& f : frames_) f = CreateScope<vulkan::Frame>(*context_);

  vk::SemaphoreCreateInfo sem_ci{};
  render_finished_.resize(swapchain_->image_count());
  for (auto& s : render_finished_) s = context_->device().createSemaphore(sem_ci);

  slang_ = CreateScope<vulkan::SlangCompiler>();

  texture_ = vulkan::Image::FromFile(*context_, *allocator_,
                                     "assets/textures/checkerboard.png");
  CK_ENGINE_ASSERT(texture_, "checkerboard.png failed to load");
  CK_ENGINE_INFO("Texture loaded: checkerboard.png {}x{}",
                 texture_->extent().width, texture_->extent().height);

  sampler_ = CreateScope<vulkan::Sampler>(*context_);

  std::array<vulkan::DescriptorPool::PoolSize, 2> pool_sizes{{
      {vk::DescriptorType::eUniformBuffer, vulkan::kFramesInFlight},
      {vk::DescriptorType::eCombinedImageSampler, 1},
  }};
  descriptor_pool_ = CreateScope<vulkan::DescriptorPool>(
      *context_, pool_sizes, vulkan::kFramesInFlight);

  std::array<vk::DescriptorSetLayoutBinding, 2> set_bindings{};
  set_bindings[0].binding = 0;
  set_bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  set_bindings[0].descriptorCount = 1;
  set_bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
  set_bindings[1].binding = 1;
  set_bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
  set_bindings[1].descriptorCount = 1;
  set_bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
  CK_ENGINE_INFO("Descriptor pool ready ({} sets max)",
                 vulkan::kFramesInFlight);

  camera_ubo_ = CreateScope<vulkan::UniformBuffer<CameraData>>(
      *allocator_, vulkan::kFramesInFlight);
  CK_ENGINE_INFO("UBO ring ready ({} frames)", vulkan::kFramesInFlight);

  // Quad geometry (centered at origin, 1x1 in world space). UVs span the full
  // texture; (0,0) at top-left consistent with Vulkan Y-down NDC.
  struct QuadVertex {
    glm::vec2 pos;
    glm::vec2 uv;
  };
  static constexpr std::array<QuadVertex, 4> kQuadVertices = {{
      {{-0.5f, -0.5f}, {0.0f, 0.0f}},
      {{ 0.5f, -0.5f}, {1.0f, 0.0f}},
      {{ 0.5f,  0.5f}, {1.0f, 1.0f}},
      {{-0.5f,  0.5f}, {0.0f, 1.0f}},
  }};
  static constexpr std::array<uint16_t, 6> kQuadIndices = {0, 1, 2, 2, 3, 0};

  quad_vbo_ = vulkan::Buffer::CreateDeviceLocal(
      *allocator_, kQuadVertices.data(), sizeof(kQuadVertices),
      vk::BufferUsageFlagBits::eVertexBuffer);
  quad_ibo_ = vulkan::Buffer::CreateDeviceLocal(
      *allocator_, kQuadIndices.data(), sizeof(kQuadIndices),
      vk::BufferUsageFlagBits::eIndexBuffer);
  CK_ENGINE_INFO("Quad VBO {} bytes / IBO {} bytes",
                 quad_vbo_->size(), quad_ibo_->size());

  vk::VertexInputBindingDescription binding{};
  binding.binding = 0;
  binding.stride = sizeof(QuadVertex);
  binding.inputRate = vk::VertexInputRate::eVertex;

  std::array<vk::VertexInputAttributeDescription, 2> attributes{};
  attributes[0].location = 0;
  attributes[0].binding = 0;
  attributes[0].format = vk::Format::eR32G32Sfloat;
  attributes[0].offset = offsetof(QuadVertex, pos);
  attributes[1].location = 1;
  attributes[1].binding = 0;
  attributes[1].format = vk::Format::eR32G32Sfloat;
  attributes[1].offset = offsetof(QuadVertex, uv);

  vulkan::VertexInput vertex_input{
      .bindings = std::span{&binding, 1},
      .attributes = std::span{attributes.data(), attributes.size()},
  };

  Material::Spec mat_spec{
      .shader_path = "assets/shaders/textured_quad.slang",
      .bindings = {set_bindings.begin(), set_bindings.end()},
      .color_format = swapchain_->format(),
      .vertex_input = vertex_input,
  };
  quad_material_ =
      CreateScope<Material>(*context_, *slang_, *descriptor_pool_, mat_spec);
  quad_material_->SetTexture(1, *texture_, *sampler_);
  for (uint32_t i = 0; i < vulkan::kFramesInFlight; ++i) {
    quad_material_->SetUniformBuffer(0, i, camera_ubo_->Handle(i),
                                     sizeof(CameraData));
  }
}

Renderer::~Renderer() {
  CK_PROFILE_FUNCTION();
  if (context_ && context_->device()) {
    context_->device().waitIdle();
    for (auto s : render_finished_) context_->device().destroySemaphore(s);
  }
}

void Renderer::OnResize(uint32_t /*width*/, uint32_t /*height*/) {
  resize_pending_ = true;
}

void Renderer::RecreateSwapchain() {
  CK_PROFILE_FUNCTION();
  // Skip if window is minimized — Application's run loop already skips render in that case,
  // but acquire/present can still trigger us before the minimize event lands.
  if (window_.GetWidth() == 0 || window_.GetHeight() == 0) return;

  vk::Device dev = context_->device();
  dev.waitIdle();

  // Image count may change across recreate -> rebuild render_finished_.
  for (auto s : render_finished_) dev.destroySemaphore(s);
  render_finished_.clear();

  swapchain_->Recreate();

  vk::SemaphoreCreateInfo sem_ci{};
  render_finished_.resize(swapchain_->image_count());
  for (auto& s : render_finished_) s = dev.createSemaphore(sem_ci);

  resize_pending_ = false;
}

void Renderer::BeginFrame() {
  CK_PROFILE_FUNCTION();
  if (resize_pending_) RecreateSwapchain();

  vk::Device dev = context_->device();
  vulkan::Frame& fr = *frames_[current_frame_];

  // Wait for previous use of this slot to finish; do NOT reset fence yet, so we can
  // bail safely on OutOfDate without leaving the fence unsignalled.
  (void)dev.waitForFences(fr.in_flight(), VK_TRUE, UINT64_MAX);

  // Write camera UBO for the current slot.
  vk::Extent2D extent = swapchain_->extent();
  camera_.SetViewport(extent.width, extent.height);
  CameraData cam{};
  cam.view_proj = camera_.view_projection();
  camera_ubo_->Write(current_frame_, cam);

  uint32_t image_index = 0;
  try {
    auto r = dev.acquireNextImageKHR(swapchain_->handle(), UINT64_MAX,
                                     fr.image_available(), nullptr);
    if (r.result == vk::Result::eSuboptimalKHR) resize_pending_ = true;
    image_index = r.value;
  } catch (const vk::OutOfDateKHRError&) {
    RecreateSwapchain();
    return;  // skip this frame; fence stays signalled
  }
  image_index_ = image_index;
  dev.resetFences(fr.in_flight());

  vk::CommandBuffer cmd = fr.command_buffer();
  cmd.reset();
  vk::CommandBufferBeginInfo begin{};
  begin.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  cmd.begin(begin);

  TransitionImage(cmd, swapchain_->images()[image_index_],
                  vk::ImageLayout::eUndefined,
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::PipelineStageFlagBits2::eTopOfPipe, {},
                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                  vk::AccessFlagBits2::eColorAttachmentWrite);

  // Time-based RGB cycle: each channel is a sine offset by 120 degrees.
  float t = std::chrono::duration<float>(
      std::chrono::steady_clock::now() - start_time_).count();
  constexpr float kTwoThirdsPi = 2.0f * std::numbers::pi_v<float> / 3.0f;
  vk::ClearColorValue clear_color{std::array<float, 4>{
      0.5f + 0.5f * std::sin(t),
      0.5f + 0.5f * std::sin(t + kTwoThirdsPi),
      0.5f + 0.5f * std::sin(t + 2.0f * kTwoThirdsPi),
      1.0f}};

  vk::RenderingAttachmentInfo color_att{};
  color_att.imageView = swapchain_->image_views()[image_index_];
  color_att.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
  color_att.loadOp = vk::AttachmentLoadOp::eClear;
  color_att.storeOp = vk::AttachmentStoreOp::eStore;
  color_att.clearValue.color = clear_color;

  vk::RenderingInfo rendering{};
  rendering.renderArea.offset = vk::Offset2D{0, 0};
  rendering.renderArea.extent = swapchain_->extent();
  rendering.layerCount = 1;
  rendering.colorAttachmentCount = 1;
  rendering.pColorAttachments = &color_att;
  cmd.beginRendering(rendering);

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, quad_material_->pipeline());
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                         quad_material_->pipeline_layout(), 0,
                         quad_material_->descriptor_set(current_frame_), {});
  vk::Buffer vbo = quad_vbo_->handle();
  vk::DeviceSize vbo_offset = 0;
  cmd.bindVertexBuffers(0, 1, &vbo, &vbo_offset);
  cmd.bindIndexBuffer(quad_ibo_->handle(), 0, vk::IndexType::eUint16);
  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(extent.width),
                        static_cast<float>(extent.height), 0.0f, 1.0f};
  cmd.setViewport(0, viewport);
  vk::Rect2D scissor{vk::Offset2D{0, 0}, extent};
  cmd.setScissor(0, scissor);
  cmd.drawIndexed(6, 1, 0, 0, 0);

  frame_active_ = true;
}

void Renderer::EndFrame() {
  CK_PROFILE_FUNCTION();
  if (!frame_active_) return;
  frame_active_ = false;

  vulkan::Frame& fr = *frames_[current_frame_];
  vk::CommandBuffer cmd = fr.command_buffer();

  cmd.endRendering();
  TransitionImage(cmd, swapchain_->images()[image_index_],
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::ImageLayout::ePresentSrcKHR,
                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                  vk::AccessFlagBits2::eColorAttachmentWrite,
                  vk::PipelineStageFlagBits2::eBottomOfPipe, {});
  cmd.end();

  vk::SemaphoreSubmitInfo wait_si{};
  wait_si.semaphore = fr.image_available();
  wait_si.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

  vk::SemaphoreSubmitInfo signal_si{};
  signal_si.semaphore = render_finished_[image_index_];
  signal_si.stageMask = vk::PipelineStageFlagBits2::eAllGraphics;

  vk::CommandBufferSubmitInfo cmd_si{};
  cmd_si.commandBuffer = cmd;

  vk::SubmitInfo2 submit{};
  submit.waitSemaphoreInfoCount = 1;
  submit.pWaitSemaphoreInfos = &wait_si;
  submit.commandBufferInfoCount = 1;
  submit.pCommandBufferInfos = &cmd_si;
  submit.signalSemaphoreInfoCount = 1;
  submit.pSignalSemaphoreInfos = &signal_si;

  context_->graphics_queue().submit2(submit, fr.in_flight());

  vk::PresentInfoKHR present{};
  present.waitSemaphoreCount = 1;
  vk::Semaphore wait_sem = render_finished_[image_index_];
  present.pWaitSemaphores = &wait_sem;
  present.swapchainCount = 1;
  vk::SwapchainKHR sc = swapchain_->handle();
  present.pSwapchains = &sc;
  present.pImageIndices = &image_index_;

  try {
    auto pres_res = context_->graphics_queue().presentKHR(present);
    if (pres_res == vk::Result::eSuboptimalKHR) resize_pending_ = true;
  } catch (const vk::OutOfDateKHRError&) {
    resize_pending_ = true;
  }

  current_frame_ = (current_frame_ + 1) % vulkan::kFramesInFlight;
}

}  // namespace ck