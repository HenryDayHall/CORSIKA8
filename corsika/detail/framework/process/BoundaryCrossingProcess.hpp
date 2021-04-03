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

  // test for doBoundaryCrossing method
  
  template <class TProcess, typename TReturn, typename... TArg>
  struct has_method_doBoundaryCrossing : public detail::has_method_signature<TReturn, TArg...> {

    using detail::has_method_signature<TReturn, TArg...>::testSignature;

    // the default value
    template <class T>
    static std::false_type test(...);

    // templated parameter option
    template <class T>
    static decltype(testSignature(&T::template doBoundaryCrossing<TArg...>)) test(
        std::nullptr_t);

    // non templated parameter option
    template <class T>
    static decltype(testSignature(&T::doBoundaryCrossing)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArg>
  bool constexpr has_method_doBoundaryCrossing_v =
    has_method_doBoundaryCrossing<TProcess, TReturn, TArg...>::value;


} // namespace corsika
