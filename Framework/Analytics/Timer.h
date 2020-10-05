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
  class timeFunction {
  private:
    typename TClock::time_point vStart;
    TDuration vDiff;

    TFunc vFunction;

  public:
    timeFunction(TFunc f)
        : vFunction(f) {}

    template <typename... TArgs>
    auto operator()(TArgs&&... args) -> std::invoke_result_t<TFunc, TArgs...> {
      vStart = TClock::now();
      auto tmp = vFunction(std::forward<TArgs>(args)...);
      vDiff = std::chrono::duration_cast<TDuration>(TClock::now() - vStart);
      return tmp;
    }

    inline TDuration getTime() const { return vDiff; }
  };

  template <typename TClass, typename TClock = std::chrono::high_resolution_clock,
            typename TDuration = std::chrono::microseconds>
  class timeProxy : public TClass {
  private:
    typename TClock::time_point vStart;
    TDuration vDiff;

    TClass& vObj;

    /*template <typename F, typename... Args>
    decltype(auto) call_func(F func, Args&&... args) {
      return (vObj.*func)(std::forward<Args>(args)...);
    }*/

  public:
    template<typename ... TArgs>
    timeProxy(TArgs args) : TClass<TArgs...>(std::forward<TArgs>(args)...)
    {}

    auto operator->() {return 2;}

    inline TDuration getTime() const { return vDiff; }
  };

} // namespace corsika::analytics
