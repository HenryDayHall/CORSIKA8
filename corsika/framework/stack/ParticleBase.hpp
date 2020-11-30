/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cstdlib> // for size_t

namespace corsika {

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
  struct ParticleBase {

  public:

    typedef StackIterator stack_iterator_type;

    ParticleBase() = default;

     // those copy constructors and assigments should never be implemented
    ParticleBase(ParticleBase&)       = delete;
    ParticleBase(ParticleBase&&)      = delete;
    ParticleBase(const ParticleBase&) = delete;

    ParticleBase operator=(ParticleBase&)       = delete;
    ParticleBase operator=(ParticleBase&&)      = delete;
    ParticleBase operator=(const ParticleBase&) = delete;


    /**
     * Delete this particle on the stack. The corresponding iterator
     * will be invalidated by this operation
     */
    void erase() { this->getIterator().getStack().erase(this->getIterator()); }

    /**
     * Method to retrieve the status of the Particle. Is it already deleted? Or not.
     */
    bool isDeleted() const { return this->getIterator().getStack().isDeleted(this->getIterator()); }

    /**
     * Add a secondary particle based on *this on the stack @param
     * args is a variadic list of input data that has to match the
     * function description in the user defined ParticleInterface::AddSecondary(...)
     */
    template <typename... TArgs>
    stack_iterator_type addSecondary(const TArgs... args) {

      return this->getStack().addSecondary(this->getIterator(), args...);
    }

    // protected: // todo should [MAY]be proteced, but don't now how to 'friend Stack'
    // Function to provide CRTP access to inheriting class (type)
    /**
     * return the corresponding StackIterator for this particle
     */
    stack_iterator_type& getIterator() {
    	return static_cast<stack_iterator_type&>(*this);
    }

    const stack_iterator_type& getIterator() const {
      return static_cast<const stack_iterator_type&>(*this);
    }

  protected:
    /**
        @name Access to underlying stack fData, these are service
        function for user classes. User code can only rely on getIndex
        and getStackData to retrieve data
        @{
    */
    auto& getStackData() {
    	return this->getIterator().getStackData();
    }

    const auto& getStackData() const {
    	return this->getIterator().getStackData();
    }

    auto& getStack() {
    	return this->getIterator().getStack();
    }

    const auto& getStack() const {
    	return this->getIterator().getStack();
    }

    /**
     * return the index number of the underlying iterator object
     */
    std::size_t getIndex() const {
    	return this->getIterator().getIndexFromIterator();
    }
    ///@}
  };

} // namespace corsika
