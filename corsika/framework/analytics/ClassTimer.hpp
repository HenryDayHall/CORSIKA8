/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

// Another possibility:
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Execute-Around_Pointer
// for a more global approach
//
// In this case here only a single function is measured via member function pointer.

#pragma once

#include <chrono>
#include <utility>

namespace corsika {

  template <class TClass>
  class ClassTimerImpl {
  protected:
    /// Default clock used for time measurement
    using TClock = std::chrono::high_resolution_clock;

    /// Internal resolution of the time measurement
    using TDuration = std::chrono::microseconds;

    /// Reference to the class object on which the function should be called
    TClass& obj_;

    /// Startpoint of time measurement
    typename TClock::time_point start_;

    /// Measured runtime of the function
    TDuration timeDiff_;
  };

  /// Measure the runtime of a single class function
  /**
   * @tparam TClassFunc Type of the member function pointer that should be wrapped
   * @tparam TFunc Actual function of the type defined in TClass
   */
  template <typename TClassFunc, TClassFunc TFunc>
  class ClassTimer;

  /// Measure the runtime of a single class function
  /** Specialisation to capture exact information about the composition of the member
   * function pointer used.
   *
   *  This class wrapes a single function and allowes the measureing of its runtime if it
   * called via the "call(...)" function
   *
   * @tparam TClass Class of the function that should be wrapped
   * @tparam TRet   Return value of the wrapped function
   * @tparam TArgs  Arguments passed to the wrapped function
   * @tparam TFuncPtr Actual function of the type defined by TRet
   * TClass::TFuncPtr(TArgs...)
   */
  template <typename TClass, typename TRet, typename... TArgs,
            TRet (TClass::*TFuncPtr)(TArgs...)>
  class ClassTimer<TRet (TClass::*)(TArgs...), TFuncPtr> : ClassTimerImpl<TClass>{
  private:
  public:
    ClassTimer(TClass& obj);

    /// Executes the wrapped function
    /** This function executes and measure the runtime of the wrapped function with the
     * highest precision available (high_resolution_clock).
     *
     * @param args Arguments are perfect forwarded to the wrapped function.
     * @return Returns the return value of the wrapped function. This value get copied
     * during the process and therefore must be copy constructible!
     */
    TRet call(TArgs... args);

    /// returns the last runtime of the wraped function accessed via call
    inline TDuration getTime() const;
  };

  /// Specialisation for member functions without return value
  template <typename TClass, typename... TArgs, void (TClass::*TFuncPtr)(TArgs...)>
  class ClassTimer<void (TClass::*)(TArgs...), TFuncPtr> : ClassTimerImpl<TClass> {
  private:
    using TClock = std::chrono::high_resolution_clock;
    using TDuration = std::chrono::microseconds;

  public:
    ClassTimer(TClass& obj);

    void call(TArgs... args);

    inline TDuration getTime() const;
  };

  /// Specialisation for const member functions
  template <typename TClass, typename TRet, typename... TArgs,
            TRet (TClass::*TFuncPtr)(TArgs...) const>
  class ClassTimer<TRet (TClass::*)(TArgs...) const, TFuncPtr> : ClassTimerImpl<TClass>{
  public:
    ClassTimer(TClass& obj);

    TRet call(TArgs... args);

    inline TDuration getTime() const;
  };

  /// Specialisation for const member functions without return value
  template <typename TClass, typename... TArgs, void (TClass::*TFuncPtr)(TArgs...) const>
  class ClassTimer<void (TClass::*)(TArgs...) const, TFuncPtr> : ClassTimerImpl<TClass> {
  public:
    ClassTimer(TClass& obj);

    void call(TArgs... args);

    inline TDuration getTime() const;
  };

} // namespace corsika

#include <corsika/detail/framework/analytics/ClassTimer.inl>