/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

// Another possibility:
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Execute-Around_Pointer

#pragma once

#include <chrono>
#include <utility>

namespace corsika::analytics {

  template <typename TFunc, typename TClock = std::chrono::high_resolution_clock,
            typename TDuration = std::chrono::microseconds>
  class FunctionTimer {
  private:
    typename TClock::time_point start_;
    TDuration timeDiff_;

    TFunc function_;

  public:
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
