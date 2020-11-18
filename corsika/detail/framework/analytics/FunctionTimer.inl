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

  template <typename TFunc, typename TTime>
  FunctionTimer<TFunc, TTime>::FunctionTimer(TFunc f)
      : function_(f) {}

  template <typename TFunc, typename TTime>
  template <typename... TArgs>
  auto FunctionTimer<TFunc, TTime>::operator()(TArgs&&... args)
      -> std::invoke_result_t<TFunc, TArgs...> {
    this->startTimer();
    auto tmp = function_(std::forward<TArgs>(args)...);
    this->stopTimer();
    return tmp;
  }

} // namespace corsika
