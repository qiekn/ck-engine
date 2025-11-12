#pragma once

#include "renderer/renderer_api.h"
namespace ck {

class Renderer {
public:
  static void BeginScene();
  static void EndScene();
  static void Submit(VertexArray*);

private:
  static inline RendererAPI::ApiType API() { return RendererAPI::ApiType(); }
};
}  // namespace ck
