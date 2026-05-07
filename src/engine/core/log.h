#pragma once

#include <utility>

#include "core.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "spdlog/spdlog.h"  // IWYU pragma: keep

namespace ck {

class Log {
public:
  static void Init();
  inline static Ref<spdlog::logger>& GetLogger() { return logger_; }

private:
  static Ref<spdlog::logger> logger_;
};

// Free-function log API: engine + client share one logger. Format strings
// are forwarded straight to spdlog so compile-time fmt checking still
// applies. Drop-in replacements for the old CK_ENGINE_*/CK_CLIENT_* macros.
namespace log {

template <typename... Args>
inline void trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  Log::GetLogger()->trace(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  Log::GetLogger()->debug(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  Log::GetLogger()->info(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  Log::GetLogger()->warn(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  Log::GetLogger()->error(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline void fatal(spdlog::format_string_t<Args...> fmt, Args&&... args) {
  Log::GetLogger()->critical(fmt, std::forward<Args>(args)...);
}

}  // namespace log
}  // namespace ck

// GLM stream helpers (kept from the macro-era log.h).
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

#define CK_ENABLE_ASSERTS
#ifdef CK_ENABLE_ASSERTS
#define CK_ASSERT(x, ...)                              \
  if (!(x)) {                                          \
    ::ck::log::error("Assertion failed: {}", __VA_ARGS__); \
    std::abort();                                      \
  }
#else
#define CK_ASSERT(x, ...)
#endif