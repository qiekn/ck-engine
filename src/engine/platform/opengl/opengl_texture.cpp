#include "opengl_texture.h"

#include "core/log.h"
#include "glad/gl.h"
#include "stb_image.h"

namespace ck {
OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : path_(path) {
  stbi_set_flip_vertically_on_load(1);

  int width, height, channels;

  stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
  CK_ENGINE_ASSERT(data, "Failed to load image: {}", path);
  CK_ENGINE_WARN("Loaded image: {} ({}x{}, {} channels)", path, width, height, channels);

  width_ = width;
  height_ = height;

  GLenum internal_format = 0, data_format = 0;
  if (channels == 4) {
    internal_format = GL_RGBA8;
    data_format = GL_RGBA;
  } else if (channels == 3) {
    internal_format = GL_RGB8;
    data_format = GL_RGB;
  }

  CK_ENGINE_ASSERT(internal_format && data_format,
                   "Unsupported image format! Channels: {} (only RGB and RGBA are supported)",
                   channels);

  glGenTextures(1, &renderer_id_);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Minify
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Magnify
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Fix alignment issue for RGB textures (3 bytes per pixel)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width_, height_, 0, data_format, GL_UNSIGNED_BYTE,
               data);

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {}

void OpenGLTexture2D::Bind(uint32_t slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);
}
}  // namespace ck
