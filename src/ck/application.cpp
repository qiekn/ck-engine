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
#include "renderer/orthographic_camera.h"
#include "renderer/render_command.h"
#include "renderer/renderer.h"
#include "renderer/vertex_array.h"

namespace ck {

Application* Application::instance_ = nullptr;

Application::Application() : camera_(-1.6f, 1.6f, -0.9f, 0.9f) {
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

  /*─────────────────────────────────────┐
  │              Triangles               │
  └──────────────────────────────────────*/

  std::shared_ptr<VertexBuffer> vertex_buffer = VertexBuffer::Create(vertices, sizeof(vertices));
  BufferLayout layout = {{ShaderDataType::kFloat3, "a_position"},
                         {ShaderDataType::kFloat4, "a_color"}};
  vertex_buffer->SetLayout(layout);

  uint32_t indices[3] = {0, 1, 2};
  std::shared_ptr<IndexBuffer> index_buffer =
      IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

  vertex_array_ = VertexArray::Create();
  vertex_array_->AddVertexBuffer(vertex_buffer);
  vertex_array_->SetIndexBuffer(index_buffer);

  std::string vertex_source = R"(
    #version 330 core

    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec4 a_color;

    uniform mat4 u_view_projection;

    out vec3 v_position;
    out vec4 v_color;

    void main() {
      v_position = a_position;
      v_color = a_color;
      gl_Position = u_view_projection * vec4(a_position, 1.0);
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

  /*─────────────────────────────────────┐
  │                Square                │
  └──────────────────────────────────────*/

  // clang-format off
  float square_vertices[] = {
    -0.75f, -0.75f, 0.0f, 
     0.75f, -0.75f, 0.0f, 
     0.75f,  0.75f, 0.0f, 
    -0.75f,  0.75f, 0.0f, 
  };
  // clang-format on

  std::shared_ptr<VertexBuffer> square_vb =
      VertexBuffer::Create(square_vertices, sizeof(square_vertices));
  square_vb->SetLayout({{ShaderDataType::kFloat3, "a_position"}});

  uint32_t square_indices[] = {0, 1, 2, 2, 3, 0};
  std::shared_ptr<IndexBuffer> square_ib =
      IndexBuffer::Create(square_indices, sizeof(square_indices) / sizeof(uint32_t));

  square_va_ = VertexArray::Create();
  square_va_->AddVertexBuffer(square_vb);
  square_va_->SetIndexBuffer(square_ib);

  std::string blue_shader_vertex_source = R"(
    #version 330 core

    layout(location = 0) in vec3 a_position;

    uniform mat4 u_view_projection;

    out vec3 v_position;

    void main() {
      v_position = a_position;
      gl_Position = u_view_projection * vec4(a_position, 1.0);
    }
  )";

  std::string blue_shader_fragment_source = R"(
    #version 330 core

    layout(location = 0) out vec4 color;

    in vec3 v_position;

    void main() {
      color = vec4(0.2, 0.3, 0.8, 1.0);
    }
  )";

  blue_shader_ = std::make_unique<Shader>(blue_shader_vertex_source, blue_shader_fragment_source);
}

Application::~Application() {}

void Application::Run() {
  while (running_) {
    RenderCommand::SetClearColor({0.25f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    camera_.SetPosition({0.0f, 0.0f, 0.0f});
    camera_.SetRotation(45.0f);

    Renderer::BeginScene(camera_);

    Renderer::Submit(blue_shader_.get(), square_va_.get());
    Renderer::Submit(shader_.get(), vertex_array_.get());

    Renderer::EndScene();

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
