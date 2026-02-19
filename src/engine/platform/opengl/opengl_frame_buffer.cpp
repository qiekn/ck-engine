#include "opengl_frame_buffer.h"

#include "glad/gl.h"

namespace ck {
static const uint32_t s_max_frame_buffer_size = 8192;

namespace utils {

static GLenum TextureTarget(bool multisampled) {
  return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static void CreateTextures(bool multisampled, uint32_t* out_id, uint32_t count) {
  glCreateTextures(TextureTarget(multisampled), count, out_id);
}

static void BindTexture(bool multisampled, uint32_t id) {
  glBindTexture(TextureTarget(multisampled), id);
}

static void AttachColorTexture(uint32_t id, int samples, GLenum internal_format, GLenum format,
                               uint32_t width, uint32_t height, int index) {
  bool multisampled = samples > 1;
  if (multisampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height,
                            GL_FALSE);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE,
                 nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                         TextureTarget(multisampled), id, 0);
}

static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachment_type,
                               uint32_t width, uint32_t height) {
  bool multisampled = samples > 1;
  if (multisampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
  } else {
    glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, TextureTarget(multisampled), id, 0);
}

static bool IsDepthFormat(FrameBufferTextureFormat format) {
  switch (format) {
    case FrameBufferTextureFormat::Depth24Stencil8:
      return true;
    default:
      return false;
  }
}

static GLenum FBTextureFormatToGL(FrameBufferTextureFormat format) {
  switch (format) {
    case FrameBufferTextureFormat::RGBA8:
      return GL_RGBA8;
    case FrameBufferTextureFormat::RedInteger:
      return GL_RED_INTEGER;
    default:
      break;
  }
  CK_ENGINE_ASSERT(false, "Unknown FrameBufferTextureFormat");
  return 0;
}

}  // namespace utils

OpenglFrameBuffer::OpenglFrameBuffer(const FrameBufferSpecification& spec) : specification_(spec) {
  for (auto& attachment_spec : specification_.attachments.attachments) {
    if (!utils::IsDepthFormat(attachment_spec.texture_format))
      color_attachment_specifications_.emplace_back(attachment_spec);
    else
      depth_attachment_specification_ = attachment_spec;
  }

  Invalidate();
}

OpenglFrameBuffer::~OpenglFrameBuffer() {
  glDeleteFramebuffers(1, &renderer_id_);
  glDeleteTextures(static_cast<GLsizei>(color_attachments_.size()), color_attachments_.data());
  glDeleteTextures(1, &depth_attachment_);
}

void OpenglFrameBuffer::Invalidate() {
  if (renderer_id_) {
    glDeleteFramebuffers(1, &renderer_id_);
    glDeleteTextures(static_cast<GLsizei>(color_attachments_.size()), color_attachments_.data());
    glDeleteTextures(1, &depth_attachment_);

    color_attachments_.clear();
    depth_attachment_ = 0;
  }

  glCreateFramebuffers(1, &renderer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_);

  bool multisample = specification_.samples > 1;

  // Attachments
  if (!color_attachment_specifications_.empty()) {
    color_attachments_.resize(color_attachment_specifications_.size());
    utils::CreateTextures(multisample, color_attachments_.data(),
                          static_cast<uint32_t>(color_attachments_.size()));

    for (size_t i = 0; i < color_attachments_.size(); i++) {
      utils::BindTexture(multisample, color_attachments_[i]);
      switch (color_attachment_specifications_[i].texture_format) {
        case FrameBufferTextureFormat::RGBA8:
          utils::AttachColorTexture(color_attachments_[i], specification_.samples, GL_RGBA8,
                                    GL_RGBA, specification_.width, specification_.height,
                                    static_cast<int>(i));
          break;
        case FrameBufferTextureFormat::RedInteger:
          utils::AttachColorTexture(color_attachments_[i], specification_.samples, GL_R32I,
                                    GL_RED_INTEGER, specification_.width, specification_.height,
                                    static_cast<int>(i));
          break;
        default:
          break;
      }
    }
  }

  if (depth_attachment_specification_.texture_format != FrameBufferTextureFormat::None) {
    utils::CreateTextures(multisample, &depth_attachment_, 1);
    utils::BindTexture(multisample, depth_attachment_);
    switch (depth_attachment_specification_.texture_format) {
      case FrameBufferTextureFormat::Depth24Stencil8:
        utils::AttachDepthTexture(depth_attachment_, specification_.samples, GL_DEPTH24_STENCIL8,
                                  GL_DEPTH_STENCIL_ATTACHMENT, specification_.width,
                                  specification_.height);
        break;
      default:
        break;
    }
  }

  if (color_attachments_.size() > 1) {
    CK_ENGINE_ASSERT(color_attachments_.size() <= 4, "Max 4 color attachments supported");
    GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                         GL_COLOR_ATTACHMENT3};
    glDrawBuffers(static_cast<GLsizei>(color_attachments_.size()), buffers);
  } else if (color_attachments_.empty()) {
    // Only depth-pass
    glDrawBuffer(GL_NONE);
  }

  CK_ENGINE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                   "Framebuffer is incomplete!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglFrameBuffer::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_);
  glViewport(0, 0, specification_.width, specification_.height);
}

void OpenglFrameBuffer::Unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglFrameBuffer::Resize(uint32_t width, uint32_t height) {
  if (width == 0 || height == 0 || width > s_max_frame_buffer_size ||
      height > s_max_frame_buffer_size) {
    CK_ENGINE_WARN("Attempted to resize framebuffer to {}, {}", width, height);
    return;
  }
  specification_.width = width;
  specification_.height = height;
  Invalidate();
}

int OpenglFrameBuffer::ReadPixel(uint32_t attachment_index, int x, int y) {
  CK_ENGINE_ASSERT(attachment_index < color_attachments_.size(),
                   "Attachment index out of range");

  glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_index);
  int pixel_data;
  glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel_data);
  return pixel_data;
}

void OpenglFrameBuffer::ClearAttachment(uint32_t attachment_index, int value) {
  CK_ENGINE_ASSERT(attachment_index < color_attachments_.size(),
                   "Attachment index out of range");

  auto& spec = color_attachment_specifications_[attachment_index];
  glClearTexImage(color_attachments_[attachment_index], 0,
                  utils::FBTextureFormatToGL(spec.texture_format), GL_INT, &value);
}

}  // namespace ck
