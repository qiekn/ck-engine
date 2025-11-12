#pragma once

namespace ck {
enum class RendererAPI {
  kNone = 0,
  kOpenGL = 1,
};

class Renderer {
public:
  static inline RendererAPI API() { return renderer_api_; }

private:
  static RendererAPI renderer_api_;
};
}  // namespace ck
