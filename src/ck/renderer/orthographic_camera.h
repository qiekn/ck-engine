#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

namespace ck {
class OrthographicCamera {
public:
  OrthographicCamera(float left, float right, float bottom, float top);
  virtual ~OrthographicCamera();

  void SetProjection(float left, float right, float bottom, float top);
  const glm::vec3& GetPosition() const;
  void SetPosition(const glm::vec3& position);
  float GetRotation() const;
  void SetRotation(float rotation);

  const glm::mat4& GetProjectionMatrix() const { return projection_matrix_; }
  const glm::mat4& GetViewMatrix() const { return view_matrix_; }
  const glm::mat4& GetViewProjectionMatrix() const { return view_proj_matrix_; }

private:
  void RecalculateViewMatrix();

private:
  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_{1.0f};
  glm::mat4 view_proj_matrix_;

  glm::vec3 position_{0.0f, 0.0f, 0.0f};
  float rotation_ = 0.0f;
};
}  // namespace ck
