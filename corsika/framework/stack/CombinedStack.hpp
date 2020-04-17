/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/stack/Stack.hpp>

namespace corsika {

  /**
   * @class CombinedParticleInterface
   *
   * You may combine two StackData object, see class CombinedStackImpl
   * below, into one Stack, using a combined StackIterator (aka
   * CombinedParticleInterface) interface class.
   *
   * This allows to add specific information to a given Stack, could
   * be special information on a subset of entries
   * (e.g. NuclearStackExtension) or also (multi) thinning weights for
   * all particles.
   *
   * Many Stacks can be combined into more complex object.
   *
   * The two sub-stacks must both provide their independent
   * ParticleInterface classes.
   *
   */
  template <template <typename> typename ParticleInterfaceA,
            template <typename> typename ParticleInterfaceB, typename StackIterator>
  class CombinedParticleInterface
      : public ParticleInterfaceB<ParticleInterfaceA<StackIterator>> {

    //FIXME: class has no ctors, assignment operators etc.

    // template<template <typename> typename _PI>
    // template <typename StackDataType, template <typename> typename ParticleInterface>
    // template<typename T1, template <typename> typename T2> friend class Stack<T1, T2>;

    using PI_C =
        CombinedParticleInterface<ParticleInterfaceA, ParticleInterfaceB, StackIterator>;
    using PI_A = ParticleInterfaceA<StackIterator>;
    using PI_B = ParticleInterfaceB<ParticleInterfaceA<StackIterator>>;

  protected:
    using PI_B::GetIndex;     // choose B, A would also work
    using PI_B::GetStackData; // choose B, A would also work

  public:
    /**
     * @name wrapper for user functions
     * @{
     *
     * In this set of functions we call the user-provide
     * ParticleInterface SetParticleData(...) methods, either with
     * parent particle reference, or w/o.
     *
     * There is one implicit assumption here: if only one data tuple
     * is provided for SetParticleData, the data is passed on to
     * ParticleInterfaceA and the ParticleInterfaceB is
     * default-initialized. There are many occasions where this is the
     * desired behaviour, e.g. for thinning etc.
     *
     */

    template <typename... Args1>
    void SetParticleData(const std::tuple<Args1...> vA) ;

    template <typename... Args1, typename... Args2>
    void SetParticleData(const std::tuple<Args1...> vA, const std::tuple<Args2...> vB) ;

    template <typename... Args1>
    void SetParticleData(PI_C& p, const std::tuple<Args1...> vA) ;

    template <typename... Args1, typename... Args2>
    void SetParticleData(PI_C& p, const std::tuple<Args1...> vA,
                         const std::tuple<Args2...> vB) ;
    ///@}

    std::string as_string() const {
      return fmt::format("[[{}][{}]]", PI_A::as_string(), PI_B::as_string());
    }
  };

  namespace detail {

  /**
   * @class CombinedStackImpl
   *
   * Memory implementation of a combined data stack.
   *
   * The two stack data user objects Stack1Impl and Stack2Impl are
   * merged into one consistent Stack container object providing
   * access to the combined number of data entries.
   */
  template <typename Stack1Impl, typename Stack2Impl>
  class CombinedStackImpl ;

}  // namespace detail

  /**
   * Helper template alias `CombinedStack` to construct new combined
   * stack from two stack data objects and a particle readout interface.
   *
   * Note that the Stack2Impl provides only /additional/ data to
   * Stack1Impl. This is important (see above) since tuple data for
   * initialization are forwarded to Stack1Impl (first).
   */

  template <typename Stack1Impl, typename Stack2Impl, template <typename> typename _PI>
  using CombinedStack = Stack<detail::CombinedStackImpl<Stack1Impl, Stack2Impl>, _PI>;

} // namespace corsika

#include <corsika/detail/framework/stack/CombinedStack.inl>
