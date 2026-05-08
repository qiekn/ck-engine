#pragma once

#include <imgui.h>

#include "core/layer.h"
#include "events/event.h"

namespace ck {

// Engine-side overlay that owns the ImGui context and the imgui_impl_glfw +
// imgui_impl_vulkan backends. Application pushes one as an overlay and
// drives Begin / End around the per-layer OnImGuiRender pass.
//
// The actual ImGui_Impl* headers stay private to the .cpp so the umbrella
// `import ck;` doesn't drag raw Vulkan-C / ImGui-C headers across the module
// boundary; clients reach ImGui through the `ImGui::*` re-exports in ck.cppm.
class ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  ~ImGuiLayer() override;

  void OnAttach() override;
  void OnDetach() override;
  void OnEvent(Event& e) override;

  // Begin: kick a new ImGui frame (NewFrame on both backends + ImGui::NewFrame).
  // End:   ImGui::Render — builds DrawData on the CPU; the actual draw is
  //        recorded by Renderer::EndFrame via the callback registered in OnAttach.
  void Begin();
  void End();

  // When true (default), input events ImGui wants to capture get marked
  // handled so they don't bubble down to gameplay layers.
  void BlockEvents(bool block) { block_events_ = block; }

  // ImTextureID for the engine's offscreen color_target_, suitable for
  // ImGui::Image inside a ViewportPanel. Backed by a descriptor set
  // returned by ImGui_ImplVulkan_AddTexture; refreshed automatically when
  // the renderer recreates color_target (e.g. on window resize).
  ImTextureID viewport_texture_id() const { return viewport_tex_; }

private:
  void RebindViewportTexture();

  bool block_events_ = true;
  ImTextureID viewport_tex_ = 0;
};

}  // namespace ck
