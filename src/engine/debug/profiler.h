// Usage: include this header file somewhere in your code (eg. precompiled header),
// and then use like:
//
// Profiler::Get().BeginSession("Session Name");
// {
//     ProfileTimer timer("Profiled Scope Name");
//     // Code
// }
// Profiler::Get().EndSession();

#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <string>
#include <thread>

namespace ck {
// ----------------------------------------------------------------------------: Data
struct ProfileResult {
  std::string name;
  std::chrono::time_point<std::chrono::steady_clock> start, end;
  uint32_t thread_id;
};

struct ProfileSession {
  std::string name;
};

// ----------------------------------------------------------------------------: Profiler
class Profiler {
public:
  Profiler() : current_session_(nullptr), profile_count_(0) {}

  static Profiler& Get() {
    static Profiler instance;
    return instance;
  }

  void BeginSession(const std::string& name, const std::string& filepath = "result.json") {
    output_stream_.open(filepath);
    WriteHeader();
    current_session_ = new ProfileSession{name};
  }

  void EndSession() {
    WriteFooter();
    output_stream_.close();
    delete current_session_;
    current_session_ = nullptr;
    profile_count_ = 0;
  }

  void WriteProfile(const ProfileResult& result) {
    if (profile_count_++ > 0) {
      output_stream_ << ",";
    }

    std::string name = result.name;
    std::replace(name.begin(), name.end(), '"', '\'');

    int64_t duration =
        std::chrono::duration_cast<std::chrono::microseconds>(result.end - result.start).count();
    uint64_t start = std::chrono::time_point_cast<std::chrono::microseconds>(result.start)
                         .time_since_epoch()
                         .count();

    output_stream_ << "{";
    output_stream_ << "\"cat\":\"function\",";
    output_stream_ << "\"dur\":" << duration << ',';
    output_stream_ << "\"name\":\"" << name << "\",";
    output_stream_ << "\"ph\":\"X\",";
    output_stream_ << "\"pid\":0,";
    output_stream_ << "\"tid\":" << result.thread_id << ",";
    output_stream_ << "\"ts\":" << start;
    output_stream_ << "}";

    output_stream_.flush();
  }

  void WriteHeader() {
    output_stream_ << "{\"otherData\": {},\"traceEvents\":[";
    output_stream_.flush();
  }

  void WriteFooter() {
    output_stream_ << "]}";
    output_stream_.flush();
  }

private:
  ProfileSession* current_session_;
  std::ofstream output_stream_;
  int profile_count_;
};

// ----------------------------------------------------------------------------: Timer
class ProfileTimer {
public:
  using Clock = std::chrono::steady_clock;

  explicit ProfileTimer(const char* name) : name_(name), is_stopped_(false) {
    start_ = Clock::now();
  }

  ~ProfileTimer() {
    if (!is_stopped_) {
      Stop();
    }
  }

  void Stop() {
    auto end_point = Clock::now();

    uint32_t thread_id =
        static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

    Profiler::Get().WriteProfile({name_, start_, end_point, thread_id});
    is_stopped_ = true;
  }

private:
  const char* name_;
  std::chrono::time_point<Clock> start_;
  bool is_stopped_;
};
}  // namespace ck

// ----------------------------------------------------------------------------: Macros
#define CK_PROFILE 1
#if CK_PROFILE
#define CK_PROFILE_BEGIN_SESSION(name, filepath) ::ck::Profiler::Get().BeginSession(name, filepath)
#define CK_PROFILE_END_SESSION() ::ck::Profiler::Get().EndSession()

#define CK_PROFILE_SCOPE_LINE2(name, line) ::ck::ProfileTimer timer##line(name)
#define CK_PROFILE_SCOPE_LINE(name, line) CK_PROFILE_SCOPE_LINE2(name, line)
#define CK_PROFILE_SCOPE(name) CK_PROFILE_SCOPE_LINE(name, __LINE__)

#define CK_PROFILE_FUNCTION() CK_PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define CK_PROFILE_BEGIN_SESSION(name, filepath)
#define CK_PROFILE_END_SESSION()
#define CK_PROFILE_SCOPE(name)
#define CK_PROFILE_FUNCTION()
#endif
