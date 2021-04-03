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

  // test method doStack

  template <class TProcess, typename TReturn, typename... TArgs>
  struct has_method_doStack : public detail::has_method_signature<TReturn, TArgs...> {

    using detail::has_method_signature<TReturn, TArgs...>::testSignature;

    // the default value
    template <class T>
    static std::false_type test(...);

    // templated parameter option
    template <class T>
    static decltype(testSignature(&T::template doStack<TArgs...>)) test(std::nullptr_t);

    // non-templated parameter option
    template <template <typename> typename T>
    static decltype(testSignature(&T<TArgs...>::template doStack)) test(std::nullptr_t);

    template <class T>
    static decltype(testSignature(&T::doStack)) test(std::nullptr_t);

  public:
    using type = decltype(test<std::decay_t<TProcess>>(nullptr));
    static const bool value = type::value;
  };

  template <class TProcess, typename TReturn, typename... TArgs>
  bool constexpr has_method_doStack_v =
      has_method_doStack<TProcess, TReturn, TArgs...>::value;

} // namespace corsika
