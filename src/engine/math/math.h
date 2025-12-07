#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

namespace ck {
namespace math {
bool DecomposeTransform(const glm::mat4& transform, glm::vec3& out_transform,
                        glm::vec3& out_rotation, glm::vec3& out_scale);

}
}  // namespace ck
