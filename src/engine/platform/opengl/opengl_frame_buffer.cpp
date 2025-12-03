#include "opengl_frame_buffer.h"
#include "platform/opengl/opengl_texture.h"
#include "renderer/frame_buffer.h"

#include "glad/gl.h"

namespace ck {

OpenglFrameBuffer::OpenglFrameBuffer(const FrameBufferSpecification& spec) : specification_(spec) {
  Invalidate();
}

OpenglFrameBuffer::~OpenglFrameBuffer() {
  glDeleteFramebuffers(1, &renderer_id_);
  glDeleteTextures(1, &color_attachment_);
  glDeleteTextures(1, &depth_attachment_);
}

void OpenglFrameBuffer::Invalidate() {
  if (renderer_id_) {
    glDeleteFramebuffers(1, &renderer_id_);
    glDeleteTextures(1, &color_attachment_);
    glDeleteTextures(1, &depth_attachment_);
  }
  // 0.
  glCreateFramebuffers(1, &renderer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_);

  glCreateTextures(GL_TEXTURE_2D, 1, &color_attachment_);
  glBindTexture(GL_TEXTURE_2D, color_attachment_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(specification_.width),
               static_cast<GLsizei>(specification_.height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // 1.
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_, 0);

  glCreateTextures(GL_TEXTURE_2D, 1, &depth_attachment_);
  glBindTexture(GL_TEXTURE_2D, depth_attachment_);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, static_cast<GLint>(specification_.width),
                 static_cast<GLint>(specification_.height));

  // 2.
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                         depth_attachment_, 0);

  CK_ENGINE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                   "Framebuffer is incomplete!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglFrameBuffer::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_);
}

void OpenglFrameBuffer::Unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglFrameBuffer::Resize(uint32_t width, uint32_t height) {
  specification_.width = width;
  specification_.height = height;
}
}  // namespace ck
