#include "opengl_vertex_array.h"

#include "glad/gl.h"

namespace ck {

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
  switch (type) {
    case ShaderDataType::kNone:
      CK_ENGINE_ASSERT(false, "unknown ShaderDataType");
      return 0;

    case ShaderDataType::kFloat:
    case ShaderDataType::kFloat2:
    case ShaderDataType::kFloat3:
    case ShaderDataType::kFloat4:
    case ShaderDataType::kMat3:
    case ShaderDataType::kMat4:
      return GL_FLOAT;

    case ShaderDataType::kInt:
    case ShaderDataType::kInt2:
    case ShaderDataType::kInt3:
    case ShaderDataType::kInt4:
      return GL_INT;

    case ShaderDataType::kBool:
      return GL_BOOL;
  }

  CK_ENGINE_ASSERT(false, "unknown ShaderDataType");
  return 0;
};

OpenglVertexArray::OpenglVertexArray() {
  glGenVertexArrays(1, &renderer_id_);
  glBindVertexArray(renderer_id_);
}

OpenglVertexArray::~OpenglVertexArray() { glDeleteVertexArrays(1, &renderer_id_); }

void OpenglVertexArray::Bind() const { glBindVertexArray(renderer_id_); }

void OpenglVertexArray::Unbind() const { glBindVertexArray(0); }

void OpenglVertexArray::AddVertexBuffer(Ref<VertexBuffer> vertex_buffer) {
  CK_ENGINE_ASSERT(vertex_buffer->Layout().elements().size(), "vertex buffer ahs no layout");
  glBindVertexArray(renderer_id_);
  vertex_buffer->Bind();

  uint32_t index = 0;
  auto const& layout = vertex_buffer->Layout();
  for (const auto& e : layout) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, e.ComponentCount(), ShaderDataTypeToOpenGLBaseType(e.type),
                          e.normalized ? GL_TRUE : GL_FALSE, layout.stride(),
                          (const void*)(uintptr_t)e.offset);
    index++;
  }

  vertex_buffers_.push_back(std::move(vertex_buffer));
}

void OpenglVertexArray::SetIndexBuffer(Ref<IndexBuffer> index_buffer) {
  glBindVertexArray(renderer_id_);
  index_buffer->Bind();

  index_buffer_ = std::move(index_buffer);
}

}  // namespace ck
