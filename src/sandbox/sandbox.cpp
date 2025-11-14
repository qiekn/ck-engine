#include <ck.h>

#include <memory>

#include "application.h"
#include "core.h"
#include "core/deltatime.h"
#include "events/event.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "imgui.h"
#include "input.h"
#include "key_code.h"
#include "platform/opengl/opengl_shader.h"
#include "renderer/buffer.h"
#include "renderer/render_command.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

class ExampleLayer : public ck::Layer {
public:
  ExampleLayer() : Layer("Example"), camera_(-1.6f, 1.6f, -0.9f, 0.9f) {
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

    ck::Ref<ck::VertexBuffer> vertex_buffer = ck::VertexBuffer::Create(vertices, sizeof(vertices));
    ck::BufferLayout layout = {{ck::ShaderDataType::kFloat3, "a_position"},
                               {ck::ShaderDataType::kFloat4, "a_color"}};
    vertex_buffer->SetLayout(layout);

    uint32_t indices[3] = {0, 1, 2};
    ck::Ref<ck::IndexBuffer> index_buffer =
        ck::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

    vertex_array_ = ck::VertexArray::Create();
    vertex_array_->AddVertexBuffer(vertex_buffer);
    vertex_array_->SetIndexBuffer(index_buffer);

    std::string vertex_source = R"(
    #version 330 core

    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec4 a_color;

    uniform mat4 u_view_projection;
    uniform mat4 u_transform;

    out vec3 v_position;
    out vec4 v_color;

    void main() {
      v_position = a_position;
      v_color = a_color;
      gl_Position = u_view_projection * u_transform * vec4(a_position, 1.0);
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

    shader_ = ck::Shader::Create(vertex_source, fragment_source);

    /*─────────────────────────────────────┐
    │                Square                │
    └──────────────────────────────────────*/

    // clang-format off
    float square_vertices[] = {
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
      0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
      -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    ck::Ref<ck::VertexBuffer> square_vb =
        ck::VertexBuffer::Create(square_vertices, sizeof(square_vertices));
    square_vb->SetLayout({{ck::ShaderDataType::kFloat3, "a_position"},
                          {ck::ShaderDataType::kFloat2, "a_tex_coord"}});

    uint32_t square_indices[] = {0, 1, 2, 2, 3, 0};
    ck::Ref<ck::IndexBuffer> square_ib =
        ck::IndexBuffer::Create(square_indices, sizeof(square_indices) / sizeof(uint32_t));

    square_va_ = ck::VertexArray::Create();
    square_va_->AddVertexBuffer(square_vb);
    square_va_->SetIndexBuffer(square_ib);

    // Flat Color Shader
    std::string flat_color_shader_vertex_source = R"(
    #version 330 core

    layout(location = 0) in vec3 a_position;

    uniform mat4 u_view_projection;
    uniform mat4 u_transform;

    out vec3 v_position;

    void main() {
      v_position = a_position;
      gl_Position = u_view_projection * u_transform * vec4(a_position, 1.0);
    }
  )";

    std::string flat_color_shader_fragment_source = R"(
    #version 330 core

    layout(location = 0) out vec4 color;

    in vec3 v_position;

    uniform vec3 u_color;

    void main() {
      color = vec4(u_color, 1.0);
    }
  )";

    flat_color_shader_ =
        ck::Shader::Create(flat_color_shader_vertex_source, flat_color_shader_fragment_source);

    // Texture Shader
    texture_shader_ = ck::Shader::Create("assets/shaders/texture.glsl");

    cat_texture_ = ck::Texture2D::Create("assets/textures/cat.jpg");
    apple_texture_ = ck::Texture2D::Create("assets/textures/apple.png");
    std::dynamic_pointer_cast<ck::OpenGLShader>(texture_shader_)->Bind();
    std::dynamic_pointer_cast<ck::OpenGLShader>(texture_shader_)->UploadUniformInt("u_texture", 0);
  }

  void OnUpdate(ck::DeltaTime dt) override {
    if (ck::Input::IsKeyPressed(CK_KEY_LEFT)) {
      camera_position_.x -= camera_speed_ * dt;
    } else if (ck::Input::IsKeyPressed(CK_KEY_RIGHT)) {
      camera_position_.x += camera_speed_ * dt;
    }

    if (ck::Input::IsKeyPressed(CK_KEY_UP)) {
      camera_position_.y += camera_speed_ * dt;
    } else if (ck::Input::IsKeyPressed(CK_KEY_DOWN)) {
      camera_position_.y -= camera_speed_ * dt;
    }

    if (ck::Input::IsKeyPressed(CK_KEY_A)) {
      camera_rotation_ += camera_rotation_speed_ * dt;
    } else if (ck::Input::IsKeyPressed(CK_KEY_D)) {
      camera_rotation_ -= camera_rotation_speed_ * dt;
    }

    ck::RenderCommand::SetClearColor({0.25f, 0.2f, 0.2f, 1.0f});
    ck::RenderCommand::Clear();

    camera_.SetPosition(camera_position_);
    camera_.SetRotation(camera_rotation_);

    ck::Renderer::BeginScene(camera_);

    auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
    // auto blue_color = glm::vec4(0.2f, 0.3f, 0.8f, 1.0f);
    // auto red_color = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
    std::dynamic_pointer_cast<ck::OpenGLShader>(flat_color_shader_)->Bind();

    for (int y = 0; y < 20; y++) {
      for (int x = 0; x < 20; x++) {
        auto position = glm::vec3(x * 0.11f, y * 0.11f, 0.0f);
        auto transform = glm::translate(glm::mat4(1.0f), position) * scale;
        ck::Renderer::Submit(flat_color_shader_.get(), square_va_.get(), transform);
      }
    }

    cat_texture_->Bind();
    ck::Renderer::Submit(texture_shader_.get(), square_va_.get(),
                         glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, 0.0f, 0.0f)) *
                             glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

    apple_texture_->Bind();
    ck::Renderer::Submit(texture_shader_.get(), square_va_.get(),
                         glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, 0.0f, 0.0f)) *
                             glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

    // Triangle
    // ck::Renderer::Submit(shader_.get(), vertex_array_.get());

    ck::Renderer::EndScene();
  }

  void OnImGuiRender() override {
    ImGui::Begin("Camera Control");
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera_position_.x, camera_position_.y,
                camera_position_.z);
    ImGui::Text("Rotation: %.2f", camera_rotation_);

    ImGui::SliderFloat("Camera Speed", &camera_speed_, 0.1f, 5.0f);
    ImGui::End();
  }

  void OnEvent(ck::Event& event) override {}

private:
  ck::Ref<ck::VertexArray> vertex_array_;
  ck::Ref<ck::Shader> shader_;

  ck::Ref<ck::VertexArray> square_va_;
  ck::Ref<ck::Shader> flat_color_shader_;

  ck::Ref<ck::Shader> texture_shader_;

  ck::OrthographicCamera camera_;

  glm::vec3 camera_position_{0.0f};

  float camera_speed_ = 0.3f;
  float camera_rotation_ = 0.0f;
  float camera_rotation_speed_ = 1.0f;

  ck::Ref<ck::Texture2D> cat_texture_;
  ck::Ref<ck::Texture2D> apple_texture_;
};

class Sandbox : public ck::Application {
public:
  Sandbox() { PushLayer(std::make_unique<ExampleLayer>()); }
  ~Sandbox() {}
};

MAKE_APPLICATION(Sandbox)
