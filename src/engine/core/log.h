#pragma once

#include "core.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "spdlog/spdlog.h"  // IWYU pragma: keep

namespace ck {
class Log {
public:
  Log() = default;
  virtual ~Log() = default;

  static void Init();
  inline static Ref<spdlog::logger>& GetEngineLogger() { return engine_logger_; }
  inline static Ref<spdlog::logger>& GetClientLogger() { return client_logger_; }

private:
  static Ref<spdlog::logger> engine_logger_;
  static Ref<spdlog::logger> client_logger_;
};
}  // namespace ck

template <typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector) {
  return os << glm::to_string(vector);
}

template <typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix) {
  return os << glm::to_string(matrix);
}

template <typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::qua<T, Q>& quaternion) {
  return os << glm::to_string(quaternion);
}

// clang-format off
#define CK_ENGINE_TRACE(...) ::ck::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define CK_ENGINE_INFO(...)  ::ck::Log::GetEngineLogger()->info(__VA_ARGS__)
#define CK_ENGINE_WARN(...)  ::ck::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define CK_ENGINE_ERROR(...) ::ck::Log::GetEngineLogger()->error(__VA_ARGS__)
#define CK_ENGINE_FATAL(...) ::ck::Log::GetEngineLogger()->critical(__VA_ARGS__)

#define CK_CLIENT_TRACE(...) ::ck::Log::GetClientLogger()->trace(__VA_ARGS__)
#define CK_CLIENT_INFO(...)  ::ck::Log::GetClientLogger()->info(__VA_ARGS__)
#define CK_CLIENT_WARN(...)  ::ck::Log::GetClientLogger()->warn(__VA_ARGS__)
#define CK_CLIENT_ERROR(...) ::ck::Log::GetClientLogger()->error(__VA_ARGS__)
#define CK_CLIENT_FATAL(...) ::ck::Log::GetClientLogger()->critical(__VA_ARGS__)
// clang-format on

#define CK_ENABLE_ASSERTS
#ifdef CK_ENABLE_ASSERTS
#define CK_ENGINE_ASSERT(x, ...)                          \
  if (!(x)) {                                             \
    CK_ENGINE_ERROR("Assertion failed: {}", __VA_ARGS__); \
    std::abort();                                         \
  }

#define CK_CLIENT_ASSERT(x, ...)                                  \
  if (!(x)) {                                                     \
    CK_CLIENTCK_CLIENT_ERROR("Assertion failed: {}", __VA_ARGS__) \
    std::abort();                                                 \
  }
#else
#define CK_ENGINE_ASSERT(x, ...)
#define CK_CLIENT_ASSERT(x, ...)
#endif
