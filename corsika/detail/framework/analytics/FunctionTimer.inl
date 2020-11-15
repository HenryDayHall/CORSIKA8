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

namespace corsika {

  template <typename TFunc, typename TClock, typename TDuration>
  FunctionTimer<TFunc, TClock, TDuration>::FunctionTimer(TFunc f)
      : function_(f) {}

  template <typename TFunc, typename TClock, typename TDuration>
  template <typename... TArgs>
  auto FunctionTimer<TFunc, TClock, TDuration>::operator()(TArgs&&... args)
      -> std::invoke_result_t<TFunc, TArgs...> {
    start_ = TClock::now();
    auto tmp = function_(std::forward<TArgs>(args)...);
    timeDiff_ = std::chrono::duration_cast<TDuration>(TClock::now() - start_);
    return tmp;
  }

  template <typename TFunc, typename TClock, typename TDuration>
  inline TDuration FunctionTimer<TFunc, TClock, TDuration>::getTime() const {
    return timeDiff_;
  }

} // namespace corsika
