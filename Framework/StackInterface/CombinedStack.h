
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
   *
   *
   */
  template <template <typename> typename ParticleInterface,
            template <typename> typename ParticleInterfaceAdd, typename StackIterator>
  class CombinedParticleInterface
      : public ParticleInterfaceAdd<ParticleInterface<StackIterator>> {

    using C =
        CombinedParticleInterface<ParticleInterface, ParticleInterfaceAdd, StackIterator>;
    using T = ParticleInterfaceAdd<ParticleInterface<StackIterator>>;
    using I = ParticleInterface<StackIterator>;

  protected:
    using T::GetIndex;
    using T::GetStackData;

  public:
    template <typename... Args1>
    void SetParticleData(const std::tuple<Args1...> vA) {
      I::SetParticleData(vA);
      T::SetParticleData();
    }
    template <typename... Args1, typename... Args2>
    void SetParticleData(const std::tuple<Args1...> vA, const std::tuple<Args2...> vB) {
      I::SetParticleData(vA);
      T::SetParticleData(vB);
    }

    template <typename... Args1>
    void SetParticleData(C& p, const std::tuple<Args1...> vA) {
      // static_assert(MT<I>::has_not, "error");
      I::SetParticleData(static_cast<I&>(p), vA);
      T::SetParticleData(static_cast<T&>(p));
    }
    template <typename... Args1, typename... Args2>
    void SetParticleData(C& p, const std::tuple<Args1...> vA,
                         const std::tuple<Args2...> vB) {
      I::SetParticleData(static_cast<I&>(p), vA);
      T::SetParticleData(static_cast<T&>(p), vB);
    }
  };

  /**
   * Memory implementation of the most simple (stupid) particle stack object.
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

  private:
    /// the actual memory to store particle data

  }; // end class CombinedStackImpl

  template <typename Stack1Impl, typename Stack2Impl, template <typename> typename _PI>
  using CombinedStack = Stack<CombinedStackImpl<Stack1Impl, Stack2Impl>, _PI>;

} // namespace corsika::stack

#endif
