#include "opengl_uniform_buffer.h"

#include "glad/gl.h"

namespace ck {

OpenglUniformBuffer::OpenglUniformBuffer(uint32_t size, uint32_t binding) {
  glCreateBuffers(1, &renderer_id_);
  glNamedBufferData(renderer_id_, size, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, renderer_id_);
}

OpenglUniformBuffer::~OpenglUniformBuffer() {
  glDeleteBuffers(1, &renderer_id_);
}

void OpenglUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
  glNamedBufferSubData(renderer_id_, offset, size, data);
}

}  // namespace ck
