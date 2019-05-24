/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_StackIteratorinterface_h__
#define _include_StackIteratorinterface_h__

#include <corsika/stack/ParticleBase.h>

namespace corsika::stack {

  template <typename StackDataType, template <typename> typename ParticleInterface>
  class Stack; // forward decl

  template <typename StackDataType, template <typename> typename ParticleInterface>
  class SecondaryView; // forward decl

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

     The template argument `ParticleInterface` acts as a policy to provide
     readout function of Particle data from the stack. The ParticleInterface
     class must know how to retrieve information from the Stack data
     for a particle entry at any index fIndex.

     The ParticleInterface class must be written and provided by the
     user, it contains methods like <code> auto GetData() const {
     return GetStackData().GetData(GetIndex()); }</code>, where
     StackIteratorInterface::GetStackData() return a reference to the
     object storing the particle data of type StackDataType. And
     StackIteratorInterface::GetIndex() provides the iterator index to
     be readout. The StackDataType is another user-provided class to
     store data and must implement functions compatible with
     ParticleInterface, in this example StackDataType::GetData(const unsigned int
     vIndex).

     For two examples see stack_example.cc, or the
     corsika::processes::sibyll::SibStack class
  */

  template <typename StackDataType, template <typename> typename ParticleInterface,
            typename StackType = Stack<StackDataType, ParticleInterface>>
  class StackIteratorInterface
      : public ParticleInterface<
            StackIteratorInterface<StackDataType, ParticleInterface, StackType>> {

  public:
    using ParticleInterfaceType =
        ParticleInterface<corsika::stack::StackIteratorInterface<
            StackDataType, ParticleInterface, StackType>>;

    // friends are needed for access to protected methods
    friend class Stack<StackDataType,
                       ParticleInterface>; // for access to GetIndex for Stack
    friend class Stack<StackDataType&, ParticleInterface>; // for access to GetIndex
                                                           // SecondaryView : public Stack
    friend class ParticleBase<StackIteratorInterface>; // for access to GetStackDataType
    friend class SecondaryView<StackDataType,
                               ParticleInterface>; // access for SecondaryView

  private:
    unsigned int fIndex = 0;
    StackType* fData = 0; // info: Particles and StackIterators become invalid when parent
                          // Stack is copied or deleted!

    // it is not allowed to create a "dangling" stack iterator
    StackIteratorInterface() = delete;

  public:
    StackIteratorInterface(StackIteratorInterface const& vR)
        : fIndex(vR.fIndex)
        , fData(vR.fData) {}

    StackIteratorInterface& operator=(StackIteratorInterface const& vR) {
      fIndex = vR.fIndex;
      fData = vR.fData;
      return *this;
    }

    /** iterator must always point to data, with an index:
          @param data reference to the stack [rw]
          @param index index on stack
       */
    StackIteratorInterface(StackType& data, const unsigned int index)
        : fIndex(index)
        , fData(&data) {}

    /** constructor that also sets new values on particle data object
        @param data reference to the stack [rw]
        @param index index on stack
        @param args variadic list of data to initialize stack entry, this must be
       consistent with the definition of the user-provided
       ParticleInterfaceType::SetParticleData(...) function
     */
    template <typename... Args>
    StackIteratorInterface(StackType& data, const unsigned int index, const Args... args)
        : fIndex(index)
        , fData(&data) {
      (**this).SetParticleData(args...);
    }

    /** constructor that also sets new values on particle data object, including reference
        to parent particle
        @param data reference to the stack [rw]
        @param index index on stack
        @param reference to parent particle [rw]. This can be used for thinning, particle
       counting, history, etc.
        @param args variadic list of data to initialize stack entry, this must be
       consistent with the definition of the user-provided
       ParticleInterfaceType::SetParticleData(...) function
    */
    template <typename... Args>
    StackIteratorInterface(StackType& data, const unsigned int index,
                           StackIteratorInterface& parent, const Args... args)
        : fIndex(index)
        , fData(&data) {
      (**this).SetParticleData(*parent, args...);
    }

  public:
    /** @name Iterator interface
        @{
    */
    StackIteratorInterface& operator++() {
      ++fIndex;
      return *this;
    }
    StackIteratorInterface operator++(int) {
      StackIteratorInterface tmp(*this);
      ++fIndex;
      return tmp;
    }
    StackIteratorInterface operator+(int delta) {
      return StackIteratorInterface(*fData, fIndex + delta);
    }
    bool operator==(const StackIteratorInterface& rhs) { return fIndex == rhs.fIndex; }
    bool operator!=(const StackIteratorInterface& rhs) { return fIndex != rhs.fIndex; }

    /**
     * Convert iterator to value type, where value type is the user-provided particle
     * readout class
     */
    ParticleInterfaceType& operator*() {
      return static_cast<ParticleInterfaceType&>(*this);
    }
    /**
     * Convert iterator to const value type, where value type is the user-provided
     * particle readout class
     */
    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }
    ///@}

  protected:
    /**
     * @name Stack data access
     * @{
     */
    /// Get current particle index
    inline unsigned int GetIndex() const { return fIndex; }
    /// Get current particle Stack object
    inline StackType& GetStack() { return *fData; }
    /// Get current particle const Stack object
    inline const StackType& GetStack() const { return *fData; }
    /// Get current user particle StackDataType object
    inline StackDataType& GetStackData() { return fData->GetStackData(); }
    /// Get current const user particle StackDataType object
    inline const StackDataType& GetStackData() const { return fData->GetStackData(); }
    /// Get data index as mapped in Stack class
    inline unsigned int GetIndexFromIterator() const {
      return fData->GetIndexFromIterator(fIndex);
    }
    ///@}
  }; // end class StackIterator

  /**
     @class ConstStackIteratorInterface

     This is the iterator class for const-access to stack data
   */

  template <typename StackDataType, template <typename> typename ParticleInterface,
            typename StackType = Stack<StackDataType, ParticleInterface>>
  class ConstStackIteratorInterface
      : public ParticleInterface<
            ConstStackIteratorInterface<StackDataType, ParticleInterface, StackType>> {

  public:
    typedef ParticleInterface<
        ConstStackIteratorInterface<StackDataType, ParticleInterface, StackType>>
        ParticleInterfaceType;

    friend class Stack<StackDataType, ParticleInterface>;   // for access to GetIndex
    friend class ParticleBase<ConstStackIteratorInterface>; // for access to
                                                            // GetStackDataType

  private:
    unsigned int fIndex = 0;
    const StackType* fData = 0; // info: Particles and StackIterators become invalid when
                                // parent Stack is copied or deleted!

    // we don't want to allow dangling iterators to exist
    ConstStackIteratorInterface() = delete;

  public:
    ConstStackIteratorInterface(const StackType& data, const unsigned int index)
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
    ConstStackIteratorInterface operator+(int delta) {
      return ConstStackIteratorInterface(*fData, fIndex + delta);
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
    inline unsigned int GetIndex() const { return fIndex; }
    inline const StackType& GetStack() const { return *fData; }
    inline const StackDataType& GetStackData() const { return fData->GetStackData(); }
    /// Get data index as mapped in Stack class
    inline unsigned int GetIndexFromIterator() const {
      return fData->GetIndexFromIterator(fIndex);
    }
    ///@}
  }; // end class ConstStackIterator

} // namespace corsika::stack

#endif
