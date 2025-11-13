#include "opengl_texture.h"

#include "glad/gl.h"
#include "log.h"
#include "stb_image.h"

namespace ck {
OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : path_(path) {
  stbi_set_flip_vertically_on_load(1);

  int width, height, channels;

  auto* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
  CK_ENGINE_ASSERT(data, "Failed to load image: {}", path);

  width_ = width;
  height_ = height;

  glGenTextures(1, &renderer_id_);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Minify
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Magnify
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  CK_ENGINE_WARN("hello");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  CK_ENGINE_WARN("hello done");

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {}

void OpenGLTexture2D::Bind(uint32_t slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, renderer_id_);
}
}  // namespace ck
