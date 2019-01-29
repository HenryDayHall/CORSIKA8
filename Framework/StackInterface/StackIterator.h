
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_StackIterator_h__
#define _include_StackIterator_h__

#include <corsika/stack/ParticleBase.h>

#include <type_traits>

class StackData; // forward decl

namespace corsika::stack {

  template <typename StackData, template <typename> typename ParticleInterface>
  class Stack; // forward decl

  /**
     @class StackIteratorInterface 

     The StackIteratorInterface is the main interface to iterator over
     particles on a stack. At the same time StackIteratorInterface is a
     Particle object by itself, thus there is no difference between
     type and ref_type for convenience of the physicist.

     This allows to write code like
     \verbatim
     for (auto& p : theStack) { p.SetEnergy(newEnergy); }
     \endverbatim

     The template argument Stack determines the type of Stack object
     the data is stored in. A pointer to the Stack object is part of
     the StackIteratorInterface. In addition to Stack the iterator only knows
     the index fIndex in the Stack data.

     The template argument Particles acts as a policy to provide
     readout function of Particle data from the stack. The Particle
     class must know how to retrieve information from the Stack data
     for a particle entry at any index fIndex.
  */

  template <typename StackData, template <typename> typename ParticleInterface>
  class StackIteratorInterface
      : public ParticleInterface<StackIteratorInterface<StackData, ParticleInterface>> {

    typedef Stack<StackData, ParticleInterface> StackType;
    /*typedef
        typename std::conditional<std::is_const<StackData>::value,
                                  const Stack<const StackData, ParticleInterface>&,
                                  Stack<StackData, ParticleInterface>&>::type StackType;*/
    typedef ParticleInterface<StackIteratorInterface<StackData, ParticleInterface>>
        ParticleInterfaceType;

    friend class Stack<StackData, ParticleInterface>;  // for access to GetIndex
    friend class ParticleBase<StackIteratorInterface>; // for access to GetStackData

  private:
    int fIndex = 0;
    StackType* fData = 0; // info: Particles and StackIterators become invalid when parent
                          // Stack is copied or deleted!

    // it is not allowed to create a "dangling" stack iterator
    StackIteratorInterface() = delete;

  public:
    /** iterator must always point to data, with an index: 
	@param data reference to the stack [rw]
	@param index index on stack
     */
    StackIteratorInterface(StackType& data, const int index)
        : fIndex(index)
        , fData(&data) {}

    /** constructor that also sets new values on particle data object
	@param data reference to the stack [rw]
	@param index index on stack
	@param args variadic list of data to initialize stack entry, this must be consistent 
                    with the definition of the user-provided ParticleInterfaceType::SetParticleData(...) function
     */
    template <typename... Args>
    StackIteratorInterface(StackType& data, const int index, const Args... args)
        : fIndex(index)
        , fData(&data) {
      (**this).SetParticleData(args...);
    }

    /** constructor that also sets new values on particle data object, including reference
	to parent particle
	@param data reference to the stack [rw]
	@param index index on stack
	@param reference to parent particle [rw]. This can be used for thinning, particle counting, history, etc.
	@param args variadic list of data to initialize stack entry, this must be consistent 
                    with the definition of the user-provided ParticleInterfaceType::SetParticleData(...) function
    */
    template <typename... Args>
    StackIteratorInterface(StackType& data, const int index,
                           StackIteratorInterface& parent, const Args... args)
        : fIndex(index)
        , fData(&data) {
      (**this).SetParticleData(*parent, args...);
    }

  public:
    /** @name Iterator interface
     */
    ///@{
    StackIteratorInterface& operator++() {
      ++fIndex;
      return *this;
    }
    StackIteratorInterface operator++(int) {
      StackIteratorInterface tmp(*this);
      ++fIndex;
      return tmp;
    }
    bool operator==(const StackIteratorInterface& rhs) { return fIndex == rhs.fIndex; }
    bool operator!=(const StackIteratorInterface& rhs) { return fIndex != rhs.fIndex; }
    /// Convert to value type
    ParticleInterfaceType& operator*() {
      return static_cast<ParticleInterfaceType&>(*this);
    }
    /// Convert to const value type
    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }
    ///@}

  protected:
    /** @name Stack data access
     */
    ///@{
    /// Get current particle index
    inline int GetIndex() const { return fIndex; }
    /// Get current particle Stack object
    inline StackType& GetStack() { return *fData; }
    /// Get current particle const Stack object
    inline const StackType& GetStack() const { return *fData; }
    /// Get current user particle StackData object
    inline StackData& GetStackData() { return fData->GetStackData(); }
    /// Get current const user particle StackData object
    inline const StackData& GetStackData() const { return fData->GetStackData(); }
    ///@}
  }; // end class StackIterator

  /**
     @class ConstStackIteratorInterface

     This is the iterator class for const-access to stack data
   */

  template <typename StackData, template <typename> typename ParticleInterface>
  class ConstStackIteratorInterface
      : public ParticleInterface<
            ConstStackIteratorInterface<StackData, ParticleInterface>> {

    typedef Stack<StackData, ParticleInterface> StackType;
    typedef ParticleInterface<ConstStackIteratorInterface<StackData, ParticleInterface>>
        ParticleInterfaceType;

    friend class Stack<StackData, ParticleInterface>;       // for access to GetIndex
    friend class ParticleBase<ConstStackIteratorInterface>; // for access to GetStackData

  private:
    int fIndex = 0;
    const StackType* fData = 0; // info: Particles and StackIterators become invalid when
                                // parent Stack is copied or deleted!

    // we don't want to allow dangling iterators to exist
    ConstStackIteratorInterface() = delete;

  public:
    ConstStackIteratorInterface(const StackType& data, const int index)
        : fIndex(index)
        , fData(&data) {}

  /**
     @class ConstStackIteratorInterface 

     The const counterpart of StackIteratorInterface, which is used 
     for read-only iterator access on particle stack:

     \verbatim
     for (const auto& p : theStack) { E += p.GetEnergy(); }
     \endverbatim

     See documentation of StackIteratorInterface for more details.
  */
    
  public:
    /** @name Iterator interface
     */
    ///@{
    ConstStackIteratorInterface& operator++() {
      ++fIndex;
      return *this;
    }
    ConstStackIteratorInterface operator++(int) {
      ConstStackIteratorInterface tmp(*this);
      ++fIndex;
      return tmp;
    }
    bool operator==(const ConstStackIteratorInterface& rhs) {
      return fIndex == rhs.fIndex;
    }
    bool operator!=(const ConstStackIteratorInterface& rhs) {
      return fIndex != rhs.fIndex;
    }

    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }
    ///@}
    
  protected:
    /** @name Stack data access
	Only the const versions for read-only access
     */
    ///@{
    inline int GetIndex() const { return fIndex; }
    inline const StackType& GetStack() const { return *fData; }
    inline const StackData& GetStackData() const {
      return fData->GetStackData();
    }
    ///@}
  }; // end class ConstStackIterator

} // namespace corsika::stack

#endif
