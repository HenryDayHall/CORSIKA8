/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <chrono>
#include <utility>

namespace corsika::analytics {

  /// Wraps and measures the runtime of a single function type object
  /**
   *
   * @tparam TFunc funtion pointer that should be wrapped
   * @tparam TClock type of the clock that should be used for measurements
   * @tparam TDuration type of std::duration to measure the elapsed time
   */
  template <typename TFunc, typename TClock = std::chrono::high_resolution_clock,
            typename TDuration = std::chrono::microseconds>
  class FunctionTimer {
  private:
    typename TClock::time_point start_;
    TDuration timeDiff_;

    TFunc function_;

  public:
    /// Constructs the wrapper with the given functionpointer
    FunctionTimer(TFunc f)
        : function_(f) {}

    template <typename... TArgs>
    auto operator()(TArgs&&... args) -> std::invoke_result_t<TFunc, TArgs...> {
      start_ = TClock::now();
      auto tmp = function_(std::forward<TArgs>(args)...);
      timeDiff_ = std::chrono::duration_cast<TDuration>(TClock::now() - start_);
      return tmp;
    }

    inline TDuration getTime() const { return timeDiff_; }
  };

} // namespace corsika::analytics
