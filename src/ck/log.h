#pragma once

#include <memory>
#include "spdlog/spdlog.h"

namespace ck {
class Log {
public:
  Log() = default;
  virtual ~Log() = default;

  static void Init();
  inline static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return engine_logger_; }
  inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return client_logger_; }

private:
  static std::shared_ptr<spdlog::logger> engine_logger_;
  static std::shared_ptr<spdlog::logger> client_logger_;
};
}  // namespace ck

// clang-format off
#define CK_ENGINE_TRACE(...) ck::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define CK_ENGINE_INFO(...)  ck::Log::GetEngineLogger()->info(__VA_ARGS__)
#define CK_ENGINE_WARN(...)  ck::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define CK_ENGINE_ERROR(...) ck::Log::GetEngineLogger()->error(__VA_ARGS__)
#define CK_ENGINE_FATAL(...) ck::Log::GetEngineLogger()->fatal(__VA_ARGS__)

#define CK_CLIENT_TRACE(...) ck::Log::GetClientLogger()->trace(__VA_ARGS__)
#define CK_CLIENT_INFO(...)  ck::Log::GetClientLogger()->info(__VA_ARGS__)
#define CK_CLIENT_WARN(...)  ck::Log::GetClientLogger()->warn(__VA_ARGS__)
#define CK_CLIENT_ERROR(...) ck::Log::GetClientLogger()->error(__VA_ARGS__)
#define CK_CLIENT_FATAL(...) ck::Log::GetClientLogger()->fatal(__VA_ARGS__)
// clang-format on
