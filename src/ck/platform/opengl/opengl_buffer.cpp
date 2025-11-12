#include "opengl_buffer.h"

#include "glad/gl.h"

namespace ck {

/*─────────────────────────────────────┐
│            Vertex Buffer             │
└──────────────────────────────────────*/

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
  glGenBuffers(1, &renderer_id_);
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
  glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() { glDeleteBuffers(1, &renderer_id_); }

void OpenGLVertexBuffer::Bind() const { glBindBuffer(GL_ARRAY_BUFFER, renderer_id_); }

void OpenGLVertexBuffer::Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

/*─────────────────────────────────────┐
│             Index Buffer             │
└──────────────────────────────────────*/

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* vertices, uint32_t count) : count_(count) {
  glGenBuffers(1, &renderer_id_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), vertices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() { glDeleteBuffers(1, &renderer_id_); }

void OpenGLIndexBuffer::Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_); }

void OpenGLIndexBuffer::Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

}  // namespace ck
