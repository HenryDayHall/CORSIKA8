/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>


namespace corsika {

	template <template <typename> typename ParticleInterfaceA,
	            template <typename> typename ParticleInterfaceB, typename StackIterator>
    template <typename... Args1>
    void CombinedParticleInterface<ParticleInterfaceA, ParticleInterfaceB,
	                          StackIterator>::SetParticleData(const std::tuple<Args1...> vA) {
      PI_A::SetParticleData(vA);
      PI_B::SetParticleData();
    }

	template <template <typename> typename ParticleInterfaceA,
		            template <typename> typename ParticleInterfaceB, typename StackIterator>
    template <typename... Args1, typename... Args2>
    void CombinedParticleInterface<ParticleInterfaceA, ParticleInterfaceB,
    StackIterator>::SetParticleData(const std::tuple<Args1...> vA, const std::tuple<Args2...> vB) {
      PI_A::SetParticleData(vA);
      PI_B::SetParticleData(vB);
    }

	template <template <typename> typename ParticleInterfaceA,
		            template <typename> typename ParticleInterfaceB, typename StackIterator>
    template <typename... Args1>
    void CombinedParticleInterface<ParticleInterfaceA, ParticleInterfaceB,
    StackIterator>::SetParticleData(PI_C& p, const std::tuple<Args1...> vA) {
      // static_assert(MT<I>::has_not, "error");
      PI_A::SetParticleData(static_cast<PI_A&>(p), vA); // original stack
      PI_B::SetParticleData(static_cast<PI_B&>(p));     // addon stack
    }

	template <template <typename> typename ParticleInterfaceA,
		            template <typename> typename ParticleInterfaceB, typename StackIterator>
    template <typename... Args1, typename... Args2>
    void CombinedParticleInterface<ParticleInterfaceA, ParticleInterfaceB,
    StackIterator>::SetParticleData(PI_C& p, const std::tuple<Args1...> vA,
                         const std::tuple<Args2...> vB) {
      PI_A::SetParticleData(static_cast<PI_A&>(p), vA);
      PI_B::SetParticleData(static_cast<PI_B&>(p), vB);
    }


namespace detail {

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

}  // namespace detail


} // namespace corsika

