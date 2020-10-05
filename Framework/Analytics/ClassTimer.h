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

  template <typename TType, TType>
  class timeClass;

  // Specialisation for normal member functions
  template <typename TType, typename TRet, typename... TArgs,
            TRet (TType::*TFuncPtr)(TArgs...)>
  class timeClass<TRet (TType::*)(TArgs...), TFuncPtr> {
  private:
    using TClock = std::chrono::high_resolution_clock;
    using TDuration = std::chrono::microseconds;

    TType& vObj;

    typename TClock::time_point vStart;
    TDuration vDiff;

  public:
    timeClass(TType& obj)
        : vObj(obj) {}

    TRet call(TArgs&&... args) {
      vStart = TClock::now();
      auto tmp = (vObj.*TFuncPtr)(std::forward<TArgs>(args)...);
      vDiff = std::chrono::duration_cast<TDuration>(TClock::now() - vStart);
      return tmp;
    }

    inline TDuration getTime() const { return vDiff; }
  };

  // Specialisation for member functions without return value
  template <typename TType, typename... TArgs, void (TType::*TFuncPtr)(TArgs...)>
  class timeClass<void (TType::*)(TArgs...), TFuncPtr> {
  private:
    using TClock = std::chrono::high_resolution_clock;
    using TDuration = std::chrono::microseconds;

    TType& vObj;

    typename TClock::time_point vStart;
    TDuration vDiff;

  public:
    timeClass(TType& obj)
        : vObj(obj) {}

    void call(TArgs&&... args) {
      vStart = TClock::now();
      (vObj.*TFuncPtr)(std::forward<TArgs>(args)...);
      vDiff = std::chrono::duration_cast<TDuration>(TClock::now() - vStart);
      return;
    }

    inline TDuration getTime() const { return vDiff; }
  };

  // Specialisation for const member functions

  template <typename TType, typename TRet, typename... TArgs,
            TRet (TType::*TFuncPtr)(TArgs...) const>
  class timeClass<TRet (TType::*)(TArgs...) const, TFuncPtr> {
  private:
    using TClock = std::chrono::high_resolution_clock;
    using TDuration = std::chrono::microseconds;

    const TType& vObj;

    typename TClock::time_point vStart;
    TDuration vDiff;

  public:
    timeClass(TType& obj)
        : vObj(obj) {}

    TRet call(TArgs&&... args) {
      vStart = TClock::now();
      auto tmp = (vObj.*TFuncPtr)(std::forward<TArgs>(args)...);
      vDiff = std::chrono::duration_cast<TDuration>(TClock::now() - vStart);
      return tmp;
    }

    inline TDuration getTime() const { return vDiff; }
  };

  // Specialisation for const member functions without return value

  template <typename TType, typename... TArgs, void (TType::*TFuncPtr)(TArgs...) const>
  class timeClass<void (TType::*)(TArgs...) const, TFuncPtr> {
  private:
    using TClock = std::chrono::high_resolution_clock;
    using TDuration = std::chrono::microseconds;

    const TType& vObj;

    typename TClock::time_point vStart;
    TDuration vDiff;

  public:
    timeClass(TType& obj)
        : vObj(obj) {}

    void call(TArgs&&... args) {
      vStart = TClock::now();
     (vObj.*TFuncPtr)(std::forward<TArgs>(args)...);
      vDiff = std::chrono::duration_cast<TDuration>(TClock::now() - vStart);
      return ;
    }

    inline TDuration getTime() const { return vDiff; }
  };

} // namespace corsika::analytics