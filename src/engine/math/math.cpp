#include "math.h"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/epsilon.hpp"

namespace ck {
namespace math {
bool DecomposeTransform(const glm::mat4& transform, glm::vec3& position, glm::vec3& rotation,
                        glm::vec3& scale) {
  // From glm::decompose in matrix_decompose.inl
  using T = float;
  using glm::epsilon;
  using glm::epsilonEqual;
  using glm::epsilonNotEqual;
  using glm::length_t;
  using glm::mat4;
  using glm::vec3;
  using glm::vec4;

  mat4 LocalMatrix(transform);

  // Normalize the matrix.
  if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>())) {
    return false;
  }

  // First, isolate perspective.  This is the messiest.
  if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
      epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
      epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>())) {
    // Clear the perspective partition
    LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
    LocalMatrix[3][3] = static_cast<T>(1);
  }

  // Next take care of translation (easy).
  position = vec3(LocalMatrix[3]);
  LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

  vec3 Row[3], Pdum3;

  // Now get scale and shear.
  for (length_t i = 0; i < 3; ++i) {
    for (length_t j = 0; j < 3; ++j) {
      Row[i][j] = LocalMatrix[i][j];
    }
  }

  // Compute scale factors and normalize rows.
  // Guard against zero scale to prevent NaN from division by zero.
  scale.x = length(Row[0]);
  scale.y = length(Row[1]);
  scale.z = length(Row[2]);

  if (scale.x < epsilon<T>() || scale.y < epsilon<T>() || scale.z < epsilon<T>()) {
    return false;
  }

  Row[0] /= scale.x;
  Row[1] /= scale.y;
  Row[2] /= scale.z;

  // At this point, the matrix (in rows[]) is orthonormal.
  // Check for a coordinate system flip.  If the determinant
  // is -1, then negate the matrix and the scaling factors.
#if 0
  Pdum3 = cross(Row[1], Row[2]);  // v3Cross(row[1], row[2], Pdum3);
  if (dot(Row[0], Pdum3) < 0) {
    for (length_t i = 0; i < 3; i++) {
      scale[i] *= static_cast<T>(-1);
      Row[i] *= static_cast<T>(-1);
    }
  }
#endif

  rotation.y = asinf(-Row[0][2]);
  if (cos(rotation.y) != 0) {
    rotation.x = atan2f(Row[1][2], Row[2][2]);
    rotation.z = atan2f(Row[0][1], Row[0][0]);
  } else {
    rotation.x = atan2f(-Row[2][0], Row[1][1]);
    rotation.z = 0;
  }

  return true;
}

}  // namespace math
}  // namespace ck
