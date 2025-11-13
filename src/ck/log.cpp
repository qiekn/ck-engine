#include "log.h"

#include "pch.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace ck {
Ref<spdlog::logger> Log::engine_logger_;
Ref<spdlog::logger> Log::client_logger_;

void Log::Init() {
  // Docs: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
  // %^ 和 %$ 分别是颜色区域的开始和结束标识
  // 大白话就是这两个标识中间的区域会被染色
  // %T -> HH:MM:SS
  // %n -> Logger's name
  // %v -> The actual text to log
  spdlog::set_pattern("%^[%T] %n: %v%$");
  engine_logger_ = spdlog::stdout_color_mt("ENGINE");
  engine_logger_->set_level(spdlog::level::trace);

  client_logger_ = spdlog::stdout_color_mt("CLIENT");
  client_logger_->set_level(spdlog::level::trace);
}

}  // namespace ck
