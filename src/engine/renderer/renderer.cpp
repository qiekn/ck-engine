#include "renderer.h"

#include "camera.h"
#include "renderer_2d.h"
#include "renderer_3d.h"
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

constexpr vk::Format kDepthFormat = vk::Format::eD32Sfloat;

void TransitionImage(vk::CommandBuffer cmd,
                     vk::Image image,
                     vk::ImageLayout old_layout,
                     vk::ImageLayout new_layout,
                     vk::PipelineStageFlags2 src_stage,
                     vk::AccessFlags2 src_access,
                     vk::PipelineStageFlags2 dst_stage,
                     vk::AccessFlags2 dst_access,
                     vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor) {
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
  barrier.subresourceRange = vk::ImageSubresourceRange{aspect, 0, 1, 0, 1};

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
  Renderer2D::Init(*context_, *allocator_, *slang_, swapchain_->format(), kDepthFormat);
  Renderer3D::Init(*context_, *allocator_, *slang_, swapchain_->format(), kDepthFormat);
}

Renderer::~Renderer() {
  CK_PROFILE_FUNCTION();
  if (context_ && context_->device()) {
    context_->device().waitIdle();
    Renderer3D::Shutdown();
    Renderer2D::Shutdown();
    for (auto s : render_finished_) context_->device().destroySemaphore(s);
  }
}

void Renderer::OnResize(uint32_t /*width*/, uint32_t /*height*/) {
  resize_pending_ = true;
}

void Renderer::OnViewportResize(uint32_t width, uint32_t height) {
  pending_viewport_extent_ = vk::Extent2D{width, height};
}

void Renderer::SetColorTargetCallback(ColorTargetCallback cb) {
  color_target_changed_ = std::move(cb);
  // Fire once for the current target so the caller doesn't need a separate
  // "wire up the initial state" path.
  if (color_target_changed_ && color_target_) color_target_changed_();
}

void Renderer::SetActiveCamera(const glm::mat4& view_projection) {
  active_view_projection_ = view_projection;
}

void Renderer::ResetActiveCamera() {
  active_view_projection_.reset();
}

vk::ImageView Renderer::color_target_view() const {
  return color_target_ ? color_target_->view() : vk::ImageView{};
}

vk::Extent2D Renderer::color_target_extent() const {
  return color_target_ ? color_target_->extent() : vk::Extent2D{0, 0};
}

void Renderer::RecreateColorTarget(vk::Extent2D extent) {
  color_target_.reset();
  depth_target_.reset();
  vulkan::Image::CreateInfo color_ci{};
  color_ci.format = swapchain_->format();
  color_ci.extent = extent;
  // Sampled: imgui's ViewportPanel reads it as a SAMPLED_IMAGE descriptor.
  color_ci.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
  color_target_ = CreateScope<vulkan::Image>(*context_, *allocator_, color_ci);

  vulkan::Image::CreateInfo depth_ci{};
  depth_ci.format = kDepthFormat;
  depth_ci.extent = extent;
  depth_ci.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
  depth_target_ = CreateScope<vulkan::Image>(*context_, *allocator_, depth_ci);

  if (color_target_changed_) color_target_changed_();
}

void Renderer::ApplyPendingViewportResize() {
  if (pending_viewport_extent_.width == 0 || pending_viewport_extent_.height == 0) return;
  vk::Extent2D cur = color_target_ ? color_target_->extent() : vk::Extent2D{0, 0};
  if (pending_viewport_extent_ == cur) {
    pending_viewport_extent_ = vk::Extent2D{};
    return;
  }
  // color_target is shared across all in-flight frames and held by an imgui
  // descriptor set referenced inside any queued cmd buffer. waitIdle is the
  // simplest correct drain before destroying the old image. Resize fires
  // only on actual panel-size changes, so the stall is rare.
  context_->device().waitIdle();
  RecreateColorTarget(pending_viewport_extent_);
  pending_viewport_extent_ = vk::Extent2D{};
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
  ApplyPendingViewportResize();

  vk::Device dev = context_->device();
  vulkan::Frame& fr = *frames_[current_frame_];

  // Wait for previous use of this slot to finish; do NOT reset fence yet, so we can
  // bail safely on OutOfDate without leaving the fence unsignalled.
  (void)dev.waitForFences(fr.in_flight(), VK_TRUE, UINT64_MAX);

  // Camera + render area follow color_target's extent (driven by the
  // Viewport panel via OnViewportResize), not the swapchain's.
  vk::Extent2D color_extent = color_target_->extent();
  camera_.SetViewport(color_extent.width, color_extent.height);

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
  // Same story for depth: cleared every frame, so prior layout doesn't matter.
  TransitionImage(cmd, depth_target_->handle(),
                  vk::ImageLayout::eUndefined,
                  vk::ImageLayout::eDepthAttachmentOptimal,
                  vk::PipelineStageFlagBits2::eTopOfPipe, {},
                  vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                      vk::PipelineStageFlagBits2::eLateFragmentTests,
                  vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                  vk::ImageAspectFlagBits::eDepth);

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

  vk::RenderingAttachmentInfo depth_att{};
  depth_att.imageView = depth_target_->view();
  depth_att.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
  depth_att.loadOp = vk::AttachmentLoadOp::eClear;
  depth_att.storeOp = vk::AttachmentStoreOp::eDontCare;
  depth_att.clearValue.depthStencil.depth = 1.0f;
  depth_att.clearValue.depthStencil.stencil = 0;

  vk::RenderingInfo rendering{};
  rendering.renderArea.offset = vk::Offset2D{0, 0};
  rendering.renderArea.extent = color_extent;
  rendering.layerCount = 1;
  rendering.colorAttachmentCount = 1;
  rendering.pColorAttachments = &color_att;
  rendering.pDepthAttachment = &depth_att;
  cmd.beginRendering(rendering);

  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(color_extent.width),
                        static_cast<float>(color_extent.height), 0.0f, 1.0f};
  cmd.setViewport(0, viewport);
  vk::Rect2D scissor{vk::Offset2D{0, 0}, color_extent};
  cmd.setScissor(0, scissor);

  // Renderer2D batch is open between BeginScene and EndScene; layers fill
  // it via DrawQuad calls in OnUpdate. Camera matrix is fed at EndScene
  // time so layer-side camera updates land in the same frame.
  Renderer2D::BeginScene(current_frame_);
  Renderer3D::BeginScene(current_frame_);

  frame_active_ = true;
}

void Renderer::EndFrame() {
  CK_PROFILE_FUNCTION();
  if (!frame_active_) return;
  frame_active_ = false;

  vulkan::Frame& fr = *frames_[current_frame_];
  vk::CommandBuffer cmd = fr.command_buffer();

  // Pick the active camera matrix: editor pushes via SetActiveCamera each
  // OnUpdate; sandbox leaves it unset and falls back to the ortho camera_.
  glm::mat4 active_vp = active_view_projection_.value_or(camera_.view_projection());
  // 3D first (depth test+write), then 2D quads on top (no depth state).
  Renderer3D::EndScene(cmd, active_vp);
  Renderer2D::EndScene(cmd, active_vp);
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