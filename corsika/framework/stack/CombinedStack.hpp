/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

//#include <corsika/logging/Logging.h>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/stack/Stack.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

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
            template <typename> class ParticleInterfaceB, typename StackIterator>
  struct CombinedParticleInterface
      : public ParticleInterfaceB<ParticleInterfaceA<StackIterator>> {

    /**
     * @name wrapper for user functions
     * @{
     *
     * In this set of functions we call the user-provide
     * ParticleInterface setParticleData(...) methods, either with
     * parent particle reference, or w/o.
     *
     * There is one implicit assumption here: if only one data tuple
     * is provided for setParticleData, the data is passed on to
     * ParticleInterfaceA and the ParticleInterfaceB is
     * default-initialized. There are many occasions where this is the
     * desired behaviour, e.g. for thinning etc.
     *
     */

    template <typename... Args1>
    void setParticleData(const std::tuple<Args1...> vA) {
    	pi_a_type::setParticleData(vA);
    	pi_b_type::setParticleData();
    }
    template <typename... Args1, typename... Args2>
    void setParticleData(const std::tuple<Args1...> vA, const std::tuple<Args2...> vB) {
    	pi_a_type::setParticleData(vA);
        pi_b_type::setParticleData(vB);
    }

    template <typename... Args1>
    void setParticleData(PI_C& p, const std::tuple<Args1...> vA) {
      // static_assert(MT<I>::has_not, "error");
    	pi_a_type::setParticleData(static_cast<pi_a_type&>(p), vA); // original stack
        pi_b_type::setParticleData(static_cast<pi_b_type&>(p));     // addon stack
    }
    template <typename... Args1, typename... Args2>
    void setParticleData(PI_C& p, const std::tuple<Args1...> vA, const std::tuple<Args2...> vB) {

    	pi_a_type::setParticleData(static_cast<pi_a_type&>(p), vA);
        pi_b_type::setParticleData(static_cast<pi_b_type&>(p), vB);
    }
    ///@}

    std::string as_string() const {
      return fmt::format("[[{}][{}]]", PI_A::as_string(), pi_b_type::as_string());
    }

  private:
    typedef CombinedParticleInterface<ParticleInterfaceA, ParticleInterfaceB, StackIterator> pi_c_type ;
    typedef ParticleInterfaceA<StackIterator> pi_a_type;
    typedef ParticleInterfaceB<ParticleInterfaceA<StackIterator>> pi_b_type;

  protected:
    using pi_b_type::getIndex;     // choose B, A would also work
    using pi_b_type::getStackData; // choose B, A would also work

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
    void clear() {
      Stack1Impl::clear();
      Stack2Impl::clear();
    }

    unsigned int getSize() const { return Stack1Impl::getSize(); }
    unsigned int getCapacity() const { return Stack1Impl::getCapacity(); }

    /**
     *   Function to copy particle at location i1 in stack to i2
     */
    void copy(const unsigned int i1, const unsigned int i2) {
      if (i1 >= getSize() || i2 >= getSize()) {
        std::ostringstream err;
        err << "CombinedStack: trying to access data beyond size of stack!";
        throw std::runtime_error(err.str());
      }
      Stack1Impl::copy(i1, i2);
      Stack2Impl::copy(i1, i2);
    }

    /**
     *   Function to copy particle at location i2 in stack to i1
     */
    void swap(const unsigned int i1, const unsigned int i2) {
      if (i1 >= getSize() || i2 >= getSize()) {
        std::ostringstream err;
        err << "CombinedStack: trying to access data beyond size of stack!";
        throw std::runtime_error(err.str());
      }
      Stack1Impl::swap(i1, i2);
      Stack2Impl::swap(i1, i2);
    }

    void incrementSize() {
      Stack1Impl::incrementSize();
      Stack2Impl::incrementSize();
    }

    void decrementSize() {
      Stack1Impl::decrementSize();
      Stack2Impl::decrementSize();
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

  template <typename Stack1Impl, typename Stack2Impl, template <typename> typename _Pi>
  typedef  Stack<CombinedStackImpl<Stack1Impl, Stack2Impl>, Pi> combined_stack_type;

} // namespace corsika

//#include <corsika/detail/framework/stack/CombinedStack.inl>
