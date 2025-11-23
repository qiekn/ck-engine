#include <chrono>

template <typename Fn>
class Timer {
public:
  Timer(const char* name, Fn&& func) : name_(name), func_(func), stopped_(false) {
    start_timepoint_ = std::chrono::high_resolution_clock::now();
  }

  ~Timer() {
    if (!stopped_) {
      Stop();
    }
  }

  void Stop() {
    auto end_timepoint = std::chrono::high_resolution_clock::now();
    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint_)
                          .time_since_epoch()
                          .count();
    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint)
                        .time_since_epoch()
                        .count();
    stopped_ = true;
    float duration = (end - start) * 0.001f;
    func_({name_, duration});
  }

private:
  const char* name_;
  Fn func_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint_;
  bool stopped_;
};

#define PROFILE_SCOPE(name) \
  Timer timer##__LINE__(    \
      name, [&](ProfileResult profile_result) { profile_results_.push_back(profile_result); })
