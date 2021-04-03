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

  // doDecay

  template <class TProcess, typename TReturn, typename... TArgs>
  struct has_method_doDecay : public detail::has_method_signature<TReturn, TArgs...> {

    typedef std::decay_t<TProcess> process_type;

    using detail::has_method_signature<TReturn, TArgs...>::testSignature;

    // the default value
    template <class T>
    static std::false_type test(...);

    // signature of templated method
    template <class T>
    static decltype(testSignature(&T::template doDecay<TArgs...>)) test(std::nullptr_t);

    // signature of non-templated method
    template <class T>
    static decltype(testSignature(&T::doDecay)) test(std::nullptr_t);

  public:
    using type = decltype(test<process_type>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArgs>
  bool constexpr has_method_doDecay_v =
      has_method_doDecay<TProcess, TReturn, TArgs...>::value;

  // getLifetime

  template <class TProcess, typename TReturn, typename... TArgs>
  struct has_method_getLifetime : public detail::has_method_signature<TReturn, TArgs...> {

    using detail::has_method_signature<TReturn, TArgs...>::testSignature;

    // the default value
    template <class T>
    static std::false_type test(...);

    template <class T>
    static decltype(testSignature(&T::template getLifetime<TArgs...>)) test(
        std::nullptr_t);

    template <class T>
    static decltype(testSignature(&T::getLifetime)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArgs>
  bool constexpr has_method_getLifetime_v =
      has_method_getLifetime<TProcess, TReturn, TArgs...>::value;

} // namespace corsika
