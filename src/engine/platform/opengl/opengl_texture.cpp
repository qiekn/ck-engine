#include "opengl_texture.h"

#include <GL/gl.h>

#include "core/log.h"
#include "glad/gl.h"
#include "stb_image.h"

namespace ck {
OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height) : width_(width), height_(height) {
  CK_PROFILE_FUNCTION();
  internal_format_ = GL_RGBA8;
  data_format_ = GL_RGBA;

  glGenTextures(1, &renderer_id_);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : path_(path) {
  CK_PROFILE_FUNCTION();
  stbi_set_flip_vertically_on_load(1);

  int width, height, channels;

  stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
  CK_ENGINE_ASSERT(data, "Failed to load image: {}", path);
  CK_ENGINE_WARN("Loaded image: {} ({}x{}, {} channels)", path, width, height, channels);

  width_ = width;
  height_ = height;

  if (channels == 4) {
    internal_format_ = GL_RGBA8;
    data_format_ = GL_RGBA;
  } else if (channels == 3) {
    internal_format_ = GL_RGB8;
    data_format_ = GL_RGB;
  }

  CK_ENGINE_ASSERT(internal_format_ && data_format_,
                   "Unsupported image format! Channels: {} (only RGB and RGBA are supported)",
                   channels);

  glGenTextures(1, &renderer_id_);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Fix alignment issue for RGB textures (3 bytes per pixel)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format_, width_, height_, 0, data_format_,
               GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {
  CK_PROFILE_FUNCTION();
  glDeleteTextures(1, &renderer_id_);
}

void OpenGLTexture2D::Bind(uint32_t slot) const {
  CK_PROFILE_FUNCTION();
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);
}

bool OpenGLTexture2D::operator==(const Texture& other) const {
  return renderer_id_ == ((OpenGLTexture2D&)other).renderer_id_;
}

void OpenGLTexture2D::SetData(void* data, size_t size) const {
  CK_PROFILE_FUNCTION();
  uint32_t bpp = data_format_ == GL_RGBA ? 4 : 3;
  CK_ENGINE_ASSERT(size == width_ * height_ * bpp, "data must be entire texture");
  glBindTexture(GL_TEXTURE_2D, renderer_id_);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format_, width_, height_, 0, data_format_,
               GL_UNSIGNED_BYTE, data);
}
}  // namespace ck
