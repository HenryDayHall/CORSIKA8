/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_stack_combinedstack_h_
#define _include_stack_combinedstack_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::stack {

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
    void SetParticleData(const std::tuple<Args1...> vA) {
      PI_A::SetParticleData(vA);
      PI_B::SetParticleData();
    }
    template <typename... Args1, typename... Args2>
    void SetParticleData(const std::tuple<Args1...> vA, const std::tuple<Args2...> vB) {
      PI_A::SetParticleData(vA);
      PI_B::SetParticleData(vB);
    }

    template <typename... Args1>
    void SetParticleData(PI_C& p, const std::tuple<Args1...> vA) {
      // static_assert(MT<I>::has_not, "error");
      PI_A::SetParticleData(static_cast<PI_A&>(p), vA); // original stack
      PI_B::SetParticleData(static_cast<PI_B&>(p));     // addon stack
    }
    template <typename... Args1, typename... Args2>
    void SetParticleData(PI_C& p, const std::tuple<Args1...> vA,
                         const std::tuple<Args2...> vB) {
      PI_A::SetParticleData(static_cast<PI_A&>(p), vA);
      PI_B::SetParticleData(static_cast<PI_B&>(p), vB);
    }
    ///@}
  };

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
  class CombinedStackImpl : public Stack1Impl, public Stack2Impl {

  public:
    void Init() {
      Stack1Impl::Init();
      Stack2Impl::Init();
    }

    void Clear() {
      Stack1Impl::Clear();
      Stack2Impl::Clear();
    }

    unsigned int GetSize() const { return Stack1Impl::GetSize(); }
    unsigned int GetCapacity() const { return Stack1Impl::GetCapacity(); }

    /**
     *   Function to copy particle at location i1 in stack to i2
     */
    void Copy(const unsigned int i1, const unsigned int i2) {
      if (i1 >= GetSize() || i2 >= GetSize()) {
        std::ostringstream err;
        err << "CombinedStack: trying to access data beyond size of stack!";
        throw std::runtime_error(err.str());
      }
      Stack1Impl::Copy(i1, i2);
      Stack2Impl::Copy(i1, i2);
    }

    /**
     *   Function to copy particle at location i2 in stack to i1
     */
    void Swap(const unsigned int i1, const unsigned int i2) {
      if (i1 >= GetSize() || i2 >= GetSize()) {
        std::ostringstream err;
        err << "CombinedStack: trying to access data beyond size of stack!";
        throw std::runtime_error(err.str());
      }
      Stack1Impl::Swap(i1, i2);
      Stack2Impl::Swap(i1, i2);
    }

    void IncrementSize() {
      Stack1Impl::IncrementSize();
      Stack2Impl::IncrementSize();
    }

    void DecrementSize() {
      Stack1Impl::DecrementSize();
      Stack2Impl::DecrementSize();
    }

  }; // end class CombinedStackImpl

  /**
   * Helper template alias `CombinedStack` to construct new combined
   * stack from two stack data objects and a particle readout interface.
   *
   * Note that the Stack2Impl provides only /additional/ data to
   * Stack1Impl. This is important (see above) since tuple data for
   * initialization are forwarded to Stack1Impl (first).
   */

  template <typename Stack1Impl, typename Stack2Impl, template <typename> typename _PI>
  using CombinedStack = Stack<CombinedStackImpl<Stack1Impl, Stack2Impl>, _PI>;

} // namespace corsika::stack

#endif
