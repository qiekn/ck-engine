#pragma once

#include <string>

#include "renderer/texture.h"

namespace ck {
class OpenGLTexture2D : public Texture2D {
public:
  OpenGLTexture2D(const std::string& path);
  virtual ~OpenGLTexture2D();

  uint32_t Width() const override { return width_; };
  uint32_t Height() const override { return height_; };

  void Bind(uint32_t slot = 0) const override;

private:
  std::string path_;
  uint32_t renderer_id_;
  uint32_t width_;
  uint32_t height_;
};
}  // namespace ck
