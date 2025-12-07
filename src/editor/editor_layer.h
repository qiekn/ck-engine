#pragma once

#include "core/core.h"
#include "core/deltatime.h"
#include "core/layer.h"
#include "events/key_event.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "panels/scene_hierarchy_panel.h"
#include "renderer/frame_buffer.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/vertex_array.h"
#include "scene/entity.h"
#include "scene/scene.h"

namespace ck {
class EditorLayer : public Layer {
public:
  EditorLayer();
  virtual ~EditorLayer();

  virtual void OnAttach() override;
  virtual void OnDetach() override;

  void OnUpdate(DeltaTime dt) override;
  virtual void OnImGuiRender() override;
  void OnEvent(Event& e) override;

private:
  bool OnKeyPressed(KeyPressedEvent& e);

  void NewScene();

  void OpenScene();

  void SaveSceneAs();

private:
  ck::OrthographicCameraController camera_controller_;

  // TEMP
  Ref<VertexArray> square_va_;
  Ref<Shader> flat_color_shader_;
  Ref<FrameBuffer> frame_buffer_;

  Ref<Scene> active_scene_;
  Entity square_entity_;

  Entity main_camera_;
  Entity second_camera_;

  bool is_primary_camera = true;

  Ref<Texture2D> checkerboard_texture_;

  bool is_viewport_hovered_{false};
  bool is_viewprot_focused_{false};
  glm::vec2 viewport_size_{0.0f, 0.0f};

  glm::vec4 quare_color_{0.2f, 0.3f, 0.8f, 1.0f};

  int gizmo_type = -1;

  // TEMP
  glm::vec4 color_1_{0.8f, 0.2f, 0.3f, 1.0f};
  glm::vec4 color_2_{0.2f, 0.3f, 0.8f, 1.0f};
  glm::vec4 background_color_{0.25f, 0.2f, 0.2f, 1.0f};

  // Panel
  SceneHierarchyPanel scene_hierarachy_panel_;
};

}  // namespace ck
