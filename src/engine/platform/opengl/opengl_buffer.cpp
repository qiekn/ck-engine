#include "opengl_buffer.h"

#include "glad/gl.h"

namespace ck {

/*─────────────────────────────────────┐
│            Vertex Buffer             │
└──────────────────────────────────────*/

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
  CK_PROFILE_FUNCTION();
  glGenBuffers(1, &renderer_id_);
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
  glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
  CK_PROFILE_FUNCTION();
  glDeleteBuffers(1, &renderer_id_);
}

void OpenGLVertexBuffer::Bind() const {
  CK_PROFILE_FUNCTION();
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
}

void OpenGLVertexBuffer::Unbind() const {
  CK_PROFILE_FUNCTION();
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*─────────────────────────────────────┐
│             Index Buffer             │
└──────────────────────────────────────*/

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* vertices, uint32_t count) : count_(count) {
  CK_PROFILE_FUNCTION();
  glGenBuffers(1, &renderer_id_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), vertices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
  CK_PROFILE_FUNCTION();
  glDeleteBuffers(1, &renderer_id_);
}

void OpenGLIndexBuffer::Bind() const {
  CK_PROFILE_FUNCTION();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
}

void OpenGLIndexBuffer::Unbind() const {
  CK_PROFILE_FUNCTION();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}  // namespace ck
