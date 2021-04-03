/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/ProcessTraits.hpp>

namespace corsika {

  // test for doContinuous method
  
  template <class TProcess, typename TReturn, typename TArg1, typename TArg2>
  struct has_method_doContinuous : public detail::has_method_signature<TReturn, TArg1, TArg2, bool> {

    using detail::has_method_signature<TReturn, TArg1, TArg2, bool>::testSignature;

    // the default value
    template <class T>
    static std::false_type test(...);

    // templated parameter option
    template <class T>
    static decltype(testSignature(&T::template doContinuous<TArg1, TArg2>)) test(
        std::nullptr_t);

    // non templated parameter option
    template <class T>
    static decltype(testSignature(&T::doContinuous)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename TArg1, typename TArg2>
  bool constexpr has_method_doContinuous_v =
    has_method_doContinuous<TProcess, TReturn, TArg1, TArg2>::value;

  // test for getMaxStepLength method
  
  template <class TProcess, typename TReturn, typename... TArgs>
  struct has_method_getMaxStepLength
      : public detail::has_method_signature<TReturn, TArgs...> {

    using detail::has_method_signature<TReturn, TArgs...>::testSignature;

    // the default value
    template <class T>
    static std::false_type test(...);

    // templated option
    template <class T>
    static decltype(testSignature(&T::template getMaxStepLength<TArgs...>)) test(
        std::nullptr_t);

    // non templated option
    template <class T>
    static decltype(testSignature(&T::getMaxStepLength)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArgs>
  bool constexpr has_method_getMaxStepLength_v =
      has_method_getMaxStepLength<TProcess, TReturn, TArgs...>::value;

} // namespace corsika
