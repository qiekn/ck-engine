#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "renderer/orthographic_camera.h"
#include "renderer/renderer_api.h"
#include "renderer/shader.h"
namespace ck {

class Renderer {
public:
  static void BeginScene(OrthographicCamera& camera);
  static void EndScene();
  static void Submit(const Shader*, const VertexArray*,
                     const glm::mat4& transform = glm::mat4(1.0f));

  static inline RendererAPI::ApiType API() { return RendererAPI::ApiType(); }

private:
  struct SceneData {
    glm::mat4 view_projection_;
  };

  static SceneData* scene_data_;
};
}  // namespace ck
