#include "renderer.h"

#include "camera.h"
#include "renderer_2d.h"
#include "shader/slang_compiler.h"
#include "vulkan/allocator.h"
#include "vulkan/context.h"
#include "vulkan/image.h"
#include "vulkan/swapchain.h"

#include <array>
#include <cmath>
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
  RecreateColorTarget(swapchain_->extent());
  for (auto& f : frames_) f = CreateScope<vulkan::Frame>(*context_);

  vk::SemaphoreCreateInfo sem_ci{};
  render_finished_.resize(swapchain_->image_count());
  for (auto& s : render_finished_) s = context_->device().createSemaphore(sem_ci);

  slang_ = CreateScope<vulkan::SlangCompiler>();
  Renderer2D::Init(*context_, *allocator_, *slang_, swapchain_->format());
}

Renderer::~Renderer() {
  CK_PROFILE_FUNCTION();
  if (context_ && context_->device()) {
    context_->device().waitIdle();
    Renderer2D::Shutdown();
    for (auto s : render_finished_) context_->device().destroySemaphore(s);
  }
}

void Renderer::OnResize(uint32_t /*width*/, uint32_t /*height*/) {
  resize_pending_ = true;
}

void Renderer::SetColorTargetCallback(ColorTargetCallback cb) {
  color_target_changed_ = std::move(cb);
  // Fire once for the current target so the caller doesn't need a separate
  // "wire up the initial state" path.
  if (color_target_changed_ && color_target_) color_target_changed_();
}

vk::ImageView Renderer::color_target_view() const {
  return color_target_ ? color_target_->view() : vk::ImageView{};
}

void Renderer::RecreateColorTarget(vk::Extent2D extent) {
  color_target_.reset();
  vulkan::Image::CreateInfo ci{};
  ci.format = swapchain_->format();
  ci.extent = extent;
  // Sampled: imgui's ViewportPanel reads it as a SAMPLED_IMAGE descriptor.
  ci.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
  color_target_ = CreateScope<vulkan::Image>(*context_, *allocator_, ci);
  if (color_target_changed_) color_target_changed_();
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
  RecreateColorTarget(swapchain_->extent());

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

  vk::Extent2D extent = swapchain_->extent();
  camera_.SetViewport(extent.width, extent.height);

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

  // Render into the offscreen color target. We always loadOp=Clear, so we can
  // transition from Undefined every frame and let previous contents be discarded.
  TransitionImage(cmd, color_target_->handle(),
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
  color_att.imageView = color_target_->view();
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

  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(extent.width),
                        static_cast<float>(extent.height), 0.0f, 1.0f};
  cmd.setViewport(0, viewport);
  vk::Rect2D scissor{vk::Offset2D{0, 0}, extent};
  cmd.setScissor(0, scissor);

  // Renderer2D batch is open between BeginScene and EndScene; layers fill
  // it via DrawQuad calls in OnUpdate.
  Renderer2D::BeginScene(camera_, current_frame_);

  frame_active_ = true;
}

void Renderer::EndFrame() {
  CK_PROFILE_FUNCTION();
  if (!frame_active_) return;
  frame_active_ = false;

  vulkan::Frame& fr = *frames_[current_frame_];
  vk::CommandBuffer cmd = fr.command_buffer();

  Renderer2D::EndScene(cmd);
  cmd.endRendering();

  // color_target: ColorAttachmentOptimal -> ShaderReadOnlyOptimal so the
  // imgui ViewportPanel can sample it as a SAMPLED_IMAGE descriptor.
  TransitionImage(cmd, color_target_->handle(),
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::ImageLayout::eShaderReadOnlyOptimal,
                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                  vk::AccessFlagBits2::eColorAttachmentWrite,
                  vk::PipelineStageFlagBits2::eFragmentShader,
                  vk::AccessFlagBits2::eShaderSampledRead);

  // swapchain image: Undefined -> ColorAttachmentOptimal; the imgui pass
  // clears it (loadOp=Clear) and draws every pixel via the dockspace +
  // viewport panel, so we never need to preserve previous contents.
  TransitionImage(cmd, swapchain_->images()[image_index_],
                  vk::ImageLayout::eUndefined,
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::PipelineStageFlagBits2::eTopOfPipe, {},
                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                  vk::AccessFlagBits2::eColorAttachmentWrite);

  vk::ClearColorValue swapchain_clear{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};

  vk::RenderingAttachmentInfo imgui_att{};
  imgui_att.imageView = swapchain_->image_views()[image_index_];
  imgui_att.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
  imgui_att.loadOp = vk::AttachmentLoadOp::eClear;
  imgui_att.storeOp = vk::AttachmentStoreOp::eStore;
  imgui_att.clearValue.color = swapchain_clear;

  vk::RenderingInfo imgui_pass{};
  imgui_pass.renderArea.offset = vk::Offset2D{0, 0};
  imgui_pass.renderArea.extent = swapchain_->extent();
  imgui_pass.layerCount = 1;
  imgui_pass.colorAttachmentCount = 1;
  imgui_pass.pColorAttachments = &imgui_att;
  cmd.beginRendering(imgui_pass);
  if (imgui_render_) imgui_render_(cmd);
  cmd.endRendering();

  // swapchain image: ColorAttachmentOptimal -> PresentSrcKHR
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