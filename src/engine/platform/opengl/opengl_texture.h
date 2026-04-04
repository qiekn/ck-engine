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
  uint32_t GetRendererID() const override { return renderer_id_; };

  const std::string& GetPath() const override { return path_; }

  void SetData(void* data, size_t size) const override;

  void Bind(uint32_t slot = 0) const override;

  bool IsLoaded() const override { return is_loaded_; }

  bool operator==(const Texture& other) const override;

private:
  std::string path_;
  bool is_loaded_ = false;
  uint32_t renderer_id_;
  uint32_t width_;
  uint32_t height_;
  GLenum internal_format_;
  GLenum data_format_;
};
}  // namespace ck
