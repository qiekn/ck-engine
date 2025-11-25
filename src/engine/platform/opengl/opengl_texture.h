#pragma once

#include <string>

#include "glad/gl.h"
#include "renderer/texture.h"

namespace ck {
class OpenGLTexture2D : public Texture2D {
public:
  OpenGLTexture2D(uint32_t width, uint32_t height);
  OpenGLTexture2D(const std::string& path);
  virtual ~OpenGLTexture2D();

  uint32_t Width() const override { return width_; };
  uint32_t Height() const override { return height_; };

  void SetData(void* data, size_t size) const override;

  void Bind(uint32_t slot = 0) const override;

  bool operator==(const Texture& other) const override;

private:
  std::string path_;
  uint32_t renderer_id_;
  uint32_t width_;
  uint32_t height_;
  GLenum internal_format_;
  GLenum data_format_;
};
}  // namespace ck
