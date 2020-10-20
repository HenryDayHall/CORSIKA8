/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/stack/ParticleBase.hpp>

namespace corsika {

  template <typename TStackData, template <typename> typename TParticleInterface>
  class Stack; // forward decl

  template <typename TStackData, template <typename> typename TParticleInterface,
            template <class T1, template <class> class T2> class MSecondaryProducer>
  class SecondaryView; // forward decl

  template <typename TStackData, template <typename> typename TParticleInterface,
            typename StackType>
  class ConstStackIteratorInterface; // forward decl

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
     the index index_ in the Stack data.

     The template argument `TParticleInterface` acts as a policy to provide
     readout function of Particle data from the stack. The TParticleInterface
     class must know how to retrieve information from the Stack data
     for a particle entry at any index index_.

     The TParticleInterface class must be written and provided by the
     user, it contains methods like <code> auto GetData() const {
     return GetStackData().GetData(GetIndex()); }</code>, where
     StackIteratorInterface::GetStackData() return a reference to the
     object storing the particle data of type TStackData. And
     StackIteratorInterface::GetIndex() provides the iterator index to
     be readout. The TStackData is another user-provided class to
     store data and must implement functions compatible with
     TParticleInterface, in this example TStackData::GetData(const unsigned int
     vIndex).

     For two examples see stack_example.cc, or the
     corsikaes::sibyll::SibStack class
  */

  template <typename TStackData, template <typename> typename TParticleInterface,
            typename StackType = Stack<TStackData, TParticleInterface>>
  class StackIteratorInterface
      : public TParticleInterface<
            StackIteratorInterface<TStackData, TParticleInterface, StackType>> {

  public:
    using ParticleInterfaceType = ParticleInterface<
        corsika::StackIteratorInterface<StackDataType, ParticleInterface, StackType>>;

    // friends are needed for access to protected methods
    friend class Stack<TStackData,
                       TParticleInterface>; // for access to GetIndex for Stack
    friend class Stack<TStackData&, TParticleInterface>; // for access to GetIndex
                                                         // SecondaryView : public Stack
    friend class ParticleBase<StackIteratorInterface>;   // for access to GetStackData

    template <typename T1,                     // best fix this to: TStackData,
              template <typename> typename M1, // best fix this to: TParticleInterface,
              template <typename T, template <typename> typename T3> typename M2>
    friend class SecondaryView; // access grant for SecondaryView

    template <typename T, template <typename> typename ParticleInterface>
    friend class corsika::history::HistorySecondaryProducer;

    friend class ConstStackIteratorInterface<TStackData, TParticleInterface, StackType>;

  protected:
    unsigned int index_ = 0;

  private:
    StackType* data_ = 0; // info: Particles and StackIterators become invalid when parent
                          // Stack is copied or deleted!

    // it is not allowed to create a "dangling" stack iterator
    StackIteratorInterface() = delete;

  public:
    StackIteratorInterface(StackIteratorInterface&& rhs)
        : index_(std::move(rhs.index_))
        , data_(std::move(rhs.data_)) {}

    StackIteratorInterface(StackIteratorInterface const& vR)
        : index_(vR.index_)
        , data_(vR.data_) {}

    StackIteratorInterface& operator=(StackIteratorInterface const& vR) {
      index_ = vR.index_;
      data_ = vR.data_;
      return *this;
    }

    /** iterator must always point to data, with an index:
          @param data reference to the stack [rw]
          @param index index on stack
    **/
    StackIteratorInterface(StackType& data, const unsigned int index)
        : index_(index)
        , data_(&data) {}

    /** constructor that also sets new values on particle data object
        @param data reference to the stack [rw]
        @param index index on stack
        @param args variadic list of data to initialize stack entry, this must be
       consistent with the definition of the user-provided
       ParticleInterfaceType::SetParticleData(...) function
    **/
    template <typename... Args>
    StackIteratorInterface(StackType& data, const unsigned int index, const Args... args)
        : index_(index)
        , data_(&data) {
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
    **/
    template <typename... Args>
    StackIteratorInterface(StackType& data, const unsigned int index,
                           StackIteratorInterface& parent, const Args... args)
        : index_(index)
        , data_(&data) {
      (**this).SetParticleData(*parent, args...);
    }

    bool isDeleted() const { return GetStack().isDeleted(*this); }

  public:
    /** @name Iterator interface
        @{
    **/
    StackIteratorInterface& operator++() {
      do {
        ++index_;
      } while (
          GetStack().isDeleted(*this)); // this also check the allowed bounds of index_
      return *this;
    }
    StackIteratorInterface operator++(int) {
      StackIteratorInterface tmp(*this);
      do {
        ++index_;
      } while (
          GetStack().isDeleted(*this)); // this also check the allowed bounds of index_
      return tmp;
    }
    StackIteratorInterface operator+(int delta) const {
      return StackIteratorInterface(*data_, index_ + delta);
    }
    bool operator==(const StackIteratorInterface& rhs) const {
      return index_ == rhs.index_;
    }
    bool operator!=(const StackIteratorInterface& rhs) const {
      return index_ != rhs.index_;
    }
    bool operator==(
        const ConstStackIteratorInterface<TStackData, TParticleInterface, StackType>& rhs)
        const; // implement below
    bool operator!=(
        const ConstStackIteratorInterface<TStackData, TParticleInterface, StackType>& rhs)
        const; // implement below

    /**
     * Convert iterator to value type, where value type is the user-provided particle
     * readout class
     **/
    ParticleInterfaceType& operator*() {
      return static_cast<ParticleInterfaceType&>(*this);
    }

    /**
     * Convert iterator to const value type, where value type is the user-provided
     * particle readout class
     **/
    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }
    ///@}

  protected:
    /**
     * @name Stack data access
     * @{
     **/
    /// Get current particle index
    inline unsigned int GetIndex() const { return index_; }
    /// Get current particle Stack object
    inline StackType& GetStack() { return *data_; }
    /// Get current particle const Stack object
    inline const StackType& GetStack() const { return *data_; }
    /// Get current user particle TStackData object
    inline TStackData& GetStackData() { return data_->GetStackData(); }
    /// Get current const user particle TStackData object
    inline const TStackData& GetStackData() const { return data_->GetStackData(); }
    /// Get data index as mapped in Stack class
    inline unsigned int GetIndexFromIterator() const {
      return data_->GetIndexFromIterator(index_);
    }
    ///@}
  }; // end class StackIterator

  /**
     @class ConstStackIteratorInterface

     This is the iterator class for const-access to stack data
  **/

  template <typename TStackData, template <typename> typename TParticleInterface,
            typename StackType = Stack<TStackData, TParticleInterface>>
  class ConstStackIteratorInterface
      : public TParticleInterface<
            ConstStackIteratorInterface<TStackData, TParticleInterface, StackType>> {

  public:
    typedef TParticleInterface<
        ConstStackIteratorInterface<TStackData, TParticleInterface, StackType>>
        ParticleInterfaceType;

    // friends are needed for access to protected methods
    friend class Stack<TStackData,
                       TParticleInterface>; // for access to GetIndex for Stack
    friend class Stack<TStackData&, TParticleInterface>; // for access to GetIndex

    friend class ParticleBase<ConstStackIteratorInterface>; // for access to GetStackData

    template <typename T1,                     // best fix to: TStackData,
              template <typename> typename M1, // best fix to: TParticleInterface,
              template <class T2, template <class> class T3> class MSecondaryProducer>
    friend class SecondaryView; // access for SecondaryView

    friend class StackIteratorInterface<TStackData, TParticleInterface, StackType>;

    template <typename T, template <typename> typename ParticleInterface>
    friend class corsika::history::HistorySecondaryProducer;

  protected:
    unsigned int index_ = 0;

  private:
    const StackType* data_ = 0; // info: Particles and StackIterators become invalid when
                                // parent Stack is copied or deleted!

    // we don't want to allow dangling iterators to exist
    ConstStackIteratorInterface() = delete;

  public:
    ConstStackIteratorInterface(ConstStackIteratorInterface&& rhs)
        : index_(std::move(rhs.index_))
        , data_(std::move(rhs.data_)) {}

    ConstStackIteratorInterface(const StackType& data, const unsigned int index)
        : index_(index)
        , data_(&data) {}

    /**
       @class ConstStackIteratorInterface

       The const counterpart of StackIteratorInterface, which is used
       for read-only iterator access on particle stack:

       \verbatim
       for (const auto& p : theStack) { E += p.GetEnergy(); }
       \endverbatim

       See documentation of StackIteratorInterface for more details.
    **/

    bool isDeleted() const { return GetStack().isDeleted(*this); }

  public:
    /** @name Iterator interface
     **/
    ///@{
    ConstStackIteratorInterface& operator++() {
      do {
        ++index_;
      } while (
          GetStack().isDeleted(*this)); // this also check the allowed bounds of index_
      return *this;
    }
    ConstStackIteratorInterface operator++(int) {
      ConstStackIteratorInterface tmp(*this);
      do {
        ++index_;
      } while (
          GetStack().isDeleted(*this)); // this also check the allowed bounds of index_
      return tmp;
    }
    ConstStackIteratorInterface operator+(const int delta) const {
      return ConstStackIteratorInterface(*data_, index_ + delta);
    }
    bool operator==(const ConstStackIteratorInterface& rhs) const {
      return index_ == rhs.index_;
    }
    bool operator!=(const ConstStackIteratorInterface& rhs) const {
      return index_ != rhs.index_;
    }
    bool operator==(const StackIteratorInterface<TStackData, TParticleInterface,
                                                 StackType>& rhs) const {
      return index_ == rhs.index_;
    }
    bool operator!=(const StackIteratorInterface<TStackData, TParticleInterface,
                                                 StackType>& rhs) const {
      return index_ != rhs.index_;
    }

    const ParticleInterfaceType& operator*() const {
      return static_cast<const ParticleInterfaceType&>(*this);
    }
    ///@}

  protected:
    /** @name Stack data access
        Only the const versions for read-only access
    **/
    ///@{
    inline unsigned int GetIndex() const { return index_; }
    inline const StackType& GetStack() const { return *data_; }
    inline const TStackData& GetStackData() const { return data_->GetStackData(); }
    /// Get data index as mapped in Stack class
    inline unsigned int GetIndexFromIterator() const {
      return data_->GetIndexFromIterator(index_);
    }
    ///@}
  }; // end class ConstStackIterator

} // namespace corsika
