/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_particleBase_h_
#define _include_particleBase_h_

#include <type_traits>

namespace corsika::stack {

  /**
   @class ParticleBase

   The base class to define the readout of particle properties from a
   particle stack. Every stack must implement this readout via the
   ParticleBase class.

   The StackIterator template argument is derived from StackIteratorInterface, which is of
   type <code> template <typename StackData, template <typename> typename
   ParticleInterface> class StackIteratorInterface : public
   ParticleInterface<StackIteratorInterface<StackData, ParticleInterface>>
   </code>

   where StackData must refer to a Stack type, and
   ParticleInterface<StackIteratorInterface> is the corresponding particle readout class.

   Thus, StackIteratorInterface is a CRTP class, injecting the full StackIteratorInterface
   machinery into the ParticleInterface (aka ParticleBase) type!

   The declartion of a StackIteratorInterface type simultaneously declares the
   corresponding ParticleInterface type.

   Furthermore, the operator* of the StackIteratorInterface returns a
   static_cast to the ParticleInterface type, allowing a direct
   readout of the particle data from the iterator.

  */

  template <typename StackIterator>
  class ParticleBase {

  public:
    using StackIteratorType = StackIterator;
    ParticleBase() = default;

  private:
    // those copy constructors and assigments should never be implemented
    ParticleBase(ParticleBase&) = delete;
    ParticleBase operator=(ParticleBase&) = delete;
    ParticleBase(ParticleBase&&) = delete;
    ParticleBase operator=(ParticleBase&&) = delete;
    ParticleBase(const ParticleBase&) = delete;
    ParticleBase operator=(const ParticleBase&) = delete;

  public:
    /**
     * Delete this particle on the stack. The corresponding iterator
     * will be invalidated by this operation
     */
    void Delete() { GetIterator().GetStack().Delete(GetIterator()); }

    /**
     * Add a secondary particle based on *this on the stack @param
     * args is a variadic list of input data that has to match the
     * function description in the user defined ParticleInterface::AddSecondary(...)
     */
    template <typename... TArgs>
    StackIterator AddSecondary(const TArgs... args) {
      return GetStack().AddSecondary(GetIterator(), args...);
    }

    // protected: // todo should [MAY]be proteced, but don't now how to 'friend Stack'
    // Function to provide CRTP access to inheriting class (type)
    /**
     * return the corresponding StackIterator for this particle
     */
    StackIterator& GetIterator() { return static_cast<StackIterator&>(*this); }
    const StackIterator& GetIterator() const {
      return static_cast<const StackIterator&>(*this);
    }

  protected:
    /**
        @name Access to underlying stack fData, these are service
        function for user classes. User code can only rely on GetIndex
        and GetStackData to retrieve data
        @{
    */
    auto& GetStackData() { return GetIterator().GetStackData(); }
    const auto& GetStackData() const { return GetIterator().GetStackData(); }
    auto& GetStack() { return GetIterator().GetStack(); }
    const auto& GetStack() const { return GetIterator().GetStack(); }

    /**
     * return the index number of the underlying iterator object
     */
    unsigned int GetIndex() const { return GetIterator().GetIndexFromIterator(); }
    ///@}
  };

} // namespace corsika::stack

#endif
