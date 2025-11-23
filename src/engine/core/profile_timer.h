#include <chrono>
#include <ratio>

template <typename Fn>
class Timer {
public:
  using Clock = std::chrono::steady_clock;

  Timer(const char* name, Fn&& func) : name_(name), func_(func), stopped_(false) {
    start_timepoint_ = Clock::now();
  }

  ~Timer() {
    if (!stopped_) {
      Stop();
    }
  }

  void Stop() {
    auto end_timepoint = Clock::now();
    std::chrono::duration<float, std::milli> duration = end_timepoint - start_timepoint_;
    stopped_ = true;
    func_({name_, duration.count()});
  }

private:
  const char* name_;
  Fn func_;
  std::chrono::time_point<Clock> start_timepoint_;
  bool stopped_;
};

#define PROFILE_SCOPE(name) \
  Timer timer##__LINE__(    \
      name, [&](ProfileResult profile_result) { profile_results_.push_back(profile_result); })
