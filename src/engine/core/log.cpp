#include "log.h"

#include "pch.h"
#include "spdlog/sinks/stdout_color_sinks.h"  // IWYU pragma: keep
#include "spdlog/spdlog.h"

namespace ck {

Ref<spdlog::logger> Log::logger_;

void Log::Init() {
  // %T -> HH:MM:SS, %^...%$ wraps the level-colored region, %l -> level
  // tag (info/warn/...). Engine and client share the single logger so the
  // historical "ENGINE"/"CLIENT" name tag is gone.
  spdlog::set_pattern("%^[%T] [%l] %v%$");
  logger_ = spdlog::stdout_color_mt("ck");
  logger_->set_level(spdlog::level::trace);
}

}  // namespace ck