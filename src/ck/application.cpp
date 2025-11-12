#include "application.h"

#include <cstdint>
#include <memory>
#include <string>

#include "events/application_event.h"
#include "events/event.h"
#include "glad/gl.h"
#include "imgui/imgui_layer.h"
#include "log.h"
#include "renderer/buffer.h"

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
}

Application* Application::instance_ = nullptr;

Application::Application() {
  CK_ENGINE_ASSERT(Application::instance_ == nullptr, "application already exists");
  instance_ = this;
  window_ = Window::Create();
  window_->SetEventCallback(CK_BIND_EVENT(Application::OnEvent));

  auto imgui_layer = std::make_unique<ImGuiLayer>();
  imgui_layer_ = imgui_layer.get();  // 只要我们不 PopOverlay, imgui_layer_ 就不会悬空
  PushOverlay(std::move(imgui_layer));

  // clang-format off
  float vertices[] = {
    // Position         // Color
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
  };
  // clang-format on

  glGenVertexArrays(1, &vertex_array_);
  glBindVertexArray(vertex_array_);

  vertex_buffer_ = VertexBuffer::Create(vertices, sizeof(vertices));

  {
    BufferLayout layout = {{ShaderDataType::kFloat3, "a_position"},
                           {ShaderDataType::kFloat4, "a_color"}};
    vertex_buffer_->SetLayout(layout);
  }

  uint32_t index = 0;
  auto const& layout = vertex_buffer_->Layout();
  for (const auto& e : layout) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, e.ComponentCount(), ShaderDataTypeToOpenGLBaseType(e.type),
                          e.normalized ? GL_TRUE : GL_FALSE, layout.stride(),
                          (const void*)(uintptr_t)e.offset);
    index++;
  }

  // IBO - Index buffer
  uint32_t indices[3] = {0, 1, 2};
  index_buffer_ = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

  std::string vertex_source = R"(
    #version 330 core

    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec4 a_color;

    out vec3 v_position;
    out vec4 v_color;

    void main() {
      v_position = a_position;
      v_color = a_color;
      gl_Position = vec4(a_position, 1.0);
    }
  )";

  std::string fragment_source = R"(
    #version 330 core

    layout(location = 0) out vec4 color;

    in vec3 v_position;
    in vec4 v_color;

    void main() {
      color = vec4(v_position * 0.5 + 0.5, 1.0);
      color = v_color;
    }
  )";

  shader_ = std::make_unique<Shader>(vertex_source, fragment_source);
}

Application::~Application() {}

void Application::Run() {
  while (running_) {
    glClearColor(0.19f, 0.21f, 0.24f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_->Bind();
    glBindVertexArray(vertex_array_);
    glDrawElements(GL_TRIANGLES, index_buffer_->Count(), GL_UNSIGNED_INT, nullptr);

    for (auto& layer : layer_stack_) {
      layer->OnUpdate();
    }

    imgui_layer_->Begin();  // this is dirty, but it works
    for (auto& layer : layer_stack_) {
      layer->OnImGuiRender();
    }
    imgui_layer_->End();

    window_->OnUpdate();
  }
}

void Application::OnEvent(Event& e) {
  auto dispatcher = EventDispatcher(e);
  dispatcher.DispatchEvent<WindowCloseEvent>(CK_BIND_EVENT(Application::OnWindowCloseEvent));

  for (auto it = layer_stack_.end(); it != layer_stack_.begin();) {
    (*--it)->OnEvent(e);
    if (e.IsHandled()) {
      break;
    }
  }
}

bool Application::OnWindowCloseEvent(WindowCloseEvent& e) {
  running_ = false;
  return true;
}

void Application::PushLayer(std::unique_ptr<Layer> layer) {
  layer->OnAttach();
  layer_stack_.push_layer(std::move(layer));
}

void Application::PushOverlay(std::unique_ptr<Layer> layer) {
  layer->OnAttach();
  layer_stack_.push_overlay(std::move(layer));
}
}  // namespace ck
