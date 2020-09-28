/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/StackIteratorInterface.h>
#include <corsika/utl/MetaProgramming.h>

#include <stdexcept>
#include <type_traits>
#include <vector>

/**
   All classes around management of particles on a stack.
 */

namespace corsika::stack {

  /**
     This is just a forward declatation for the user-defined
     ParticleInterface, which is one of the essential template
     parameters for the Stack.

     <b>Important:</b> ParticleInterface must inherit from ParticleBase !
   */

  template <typename>
  class ParticleInterface;

  /**
     The Stack class provides (and connects) the main particle data storage machinery.

     The StackDataType type is the user-provided bare data storage
     object. This can be of any complexity, from a simple struct
     (fortran common block), to a combination of different and
     distributed data sources.

     The user-provided ParticleInterface template type is the base
     class type of the StackIteratorInterface class (CRTP) and must
     provide all functions to read single particle data from the
     StackDataType, given an 'unsigned int' index.

     The Stack implements the
     std-type begin/end function to allow integration in normal for
     loops, ranges, etc.
   */

  template <typename TStackData,
            template <typename> typename MParticleInterface>
  class Stack {
    using StackDataValueType = std::remove_reference_t<TStackData>;

  private:
    TStackData data_;           ///< this in general holds all the data and can be quite big
    std::vector<bool> deleted_; ///< bit field to flag deleted entries
  protected:
    unsigned int nDeleted_ = 0;

  private:
    Stack(Stack&) = delete; ///< since Stack can be very big, we don't want to copy it
    Stack& operator=(Stack&) =
        delete; ///< since Stack can be very big, we don't want to copy it

  public:
    /**
     * if TStackData is a reference member we *HAVE* to initialize
     * it in the constructor, this is typically needed for SecondaryView
     */
    template <typename _ = TStackData, typename = utl::enable_if<std::is_reference<_>>>
    Stack(TStackData vD)
        : data_(vD)
        , deleted_(std::vector<bool>(data_.GetSize(), false))
        , nDeleted_(0) {}

    /**
     * This constructor takes any argument and passes it on to the
     * TStackData user class. If the user did not provide a suited
     * constructor this will fail with an error message.
     *
     * Furthermore, this is disabled with enable_if for SecondaryView
     * stacks, where the inner data container is always a reference
     * and cannot be initialized here.
     */
    template <typename... Args, typename _ = TStackData,
              typename = utl::disable_if<std::is_reference<_>>>
    Stack(Args... args)
        : data_(args...)
        , deleted_(std::vector<bool>(data_.GetSize(), false))
        , nDeleted_(0) {}

  public:
    typedef TStackData
        StackImpl; ///< this is the type of the user-provided data structure

    template <typename TSI>
    using MPIType = MParticleInterface<TSI>;

    /**
     * Via the StackIteratorInterface and ConstStackIteratorInterface
     * specialization, the type of the StackIterator
     * template class is declared for a particular stack data
     * object. Using CRTP, this also determines the type of
     * MParticleInterface template class simultaneously.
     */
    using StackIterator =
        StackIteratorInterface<StackDataValueType, MParticleInterface, Stack>;
    using ConstStackIterator =
        ConstStackIteratorInterface<StackDataValueType, MParticleInterface, Stack>;

    /**
     * this is the full type of the user-declared MParticleInterface
     */
    using ParticleInterfaceType = typename StackIterator::ParticleInterfaceType;
    /**
     * In all programming context, the object to access, copy, and
     * transport particle data is via the StackIterator
     */
    using ParticleType = StackIterator;

    // friends are needed since they need access to protected members
    friend class StackIteratorInterface<StackDataValueType, MParticleInterface, Stack>;
    friend class ConstStackIteratorInterface<StackDataValueType, MParticleInterface,
                                             Stack>;
    //friend class SecondaryView<StackDataValueType, MParticleInterface>;
    template <typename T1,                     //=TStackData,
              template <typename> typename M1, //=MParticleInterface,
                                               //             template<typename>typename M2>
              template <class T2, template <class> class T3> class MSecondaryProducer>
    friend class SecondaryView; //<TStackData,MParticleInterface,M>; // access for SecondaryView

    friend class ParticleBase<StackIterator>;

  public:
    /**
     * @name Most generic proxy methods for TStackData data_
     * @{
     */
    unsigned int GetCapacity() const { return data_.GetCapacity(); }
    unsigned int getDeleted() const { return nDeleted_; }
    unsigned int getEntries() const { return getSize() - getDeleted(); }

    template <typename... Args>
    void Clear(Args... args) {
      data_.Clear(args...);
      deleted_ = std::vector<bool>(data_.GetSize(), false);
      nDeleted_ = 0;
    }
    ///@}

  public:
    /**
     * @name These are functions required by std containers and std loops
     * @{
     */
    StackIterator begin() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[i]) break;
      }
      return StackIterator(*this, i);
    }
    StackIterator end() { return StackIterator(*this, getSize()); }
    StackIterator last() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[getSize() - 1 - i]) break;
      }
      return StackIterator(*this, getSize() - 1 - i);
    }

    ConstStackIterator begin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[i]) break;
      }
      return ConstStackIterator(*this, i);
    }
    ConstStackIterator end() const { return ConstStackIterator(*this, getSize()); }
    ConstStackIterator last() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[getSize() - 1 - i]) break;
      }
      return ConstStackIterator(*this, getSize() - 1 - i);
    }

    ConstStackIterator cbegin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[i]) break;
      }
      return ConstStackIterator(*this, 0);
    }
    ConstStackIterator cend() const { return ConstStackIterator(*this, getSize()); }
    ConstStackIterator clast() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[getSize() - 1 - i]) break;
      }
      return ConstStackIterator(*this, getSize() - 1 - i);
    }
    /// @}

    StackIterator GetNextParticle() {
      while (purgeLastIfDeleted()) {}
      return last();
    }

    /**
     * increase stack size, create new particle at end of stack
     */
    template <typename... Args>
    StackIterator AddParticle(const Args... v) {
      data_.IncrementSize();
      deleted_.push_back(false);
      return StackIterator(*this, getSize() - 1, v...);
    }

  protected:
    /**
     * increase stack size, create new particle at end of stack, related to parent
     * particle/projectile
     *
     * This should only get internally called from a
     * StackIterator::AddSecondary via ParticleBase
     */
    template <typename... Args>
    StackIterator AddSecondary(StackIterator& parent, const Args... v) {
      data_.IncrementSize();
      deleted_.push_back(false);
      return StackIterator(*this, getSize() - 1, parent, v...);
    }

  public:
    void Swap(StackIterator a, StackIterator b) {
      data_.Swap(a.GetIndex(), b.GetIndex());
      std::swap(deleted_[a.GetIndex()], deleted_[b.GetIndex()]);
    }
    void Copy(StackIterator a, StackIterator b) {
      data_.Copy(a.GetIndex(), b.GetIndex());
      if (deleted_[b.GetIndex()] && !deleted_[a.GetIndex()]) nDeleted_--;
      if (!deleted_[b.GetIndex()] && deleted_[a.GetIndex()]) nDeleted_++;
      deleted_[b.GetIndex()] = deleted_[a.GetIndex()];
    }
    void Copy(ConstStackIterator a, StackIterator b) {
      data_.Copy(a.GetIndex(), b.GetIndex());
      if (deleted_[b.GetIndex()] && !deleted_[a.GetIndex()]) nDeleted_--;
      if (!deleted_[b.GetIndex()] && deleted_[a.GetIndex()]) nDeleted_++;
      deleted_[b.GetIndex()] = deleted_[a.GetIndex()];
    }

    /**
     * delete this particle
     */
  public:
    void Delete(StackIterator p) {
      if (IsEmpty()) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (deleted_[p.GetIndex()]) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since already deleted");
      }
      Delete(p.GetIndex());
    }
    /**
     * delete this particle
     */
    void Delete(ParticleInterfaceType p) { Delete(p.GetIterator()); }

    /**
     * check if there are no further non-deleted particles on stack
     */
    bool IsEmpty() { return getEntries() == 0; }

    /**
     * check if this particle was already deleted
     */
  public:
    bool isDeleted(const StackIterator& p) { return isDeleted(p.GetIndex()); }
    bool isDeleted(const ConstStackIterator& p) const { return isDeleted(p.GetIndex()); }
    bool isDeleted(const ParticleInterfaceType& p) { return isDeleted(p.GetIterator()); }

    /**
     * Function to ultimatively remove the last entry from the stack,
     * if it was marked as deleted before. If this is not the case,
     * the function will just return false and do nothing.
     */
    bool purgeLastIfDeleted() {
      if (!deleted_.back())
        return false; // the last particle is not marked for deletion. Do nothing.
      data_.DecrementSize();
      nDeleted_--;
      deleted_.pop_back();
      return true;
    }

    /**
     * Function to ultimatively remove all entries from the stack
     * marked as deleted.
     *
     * Careful: this will re-order the entries on the stack, since
     * "gaps" in the stack are filled with entries from the back
     * (copied).
     */
    void purge() {
      unsigned int iStackFront = 0;
      unsigned int iStackBack = getSize() - 1;
      for (unsigned int iDeleted = 0; iDeleted < getDeleted(); ++iDeleted) {
        // search first delete entry on stack
        while (!deleted_[iStackFront]) { iStackFront++; }
        // search for last non-deleted particle on stack
        while (deleted_[iStackBack]) { iStackBack--; }
        // copy entry from iStackBack to iStackFront
        data_.Copy(iStackBack, iStackFront);
        data_.DecrementSize();
      }
      deleted_.clear();
      nDeleted_ = 0;
    }

  protected:
    unsigned int getSize() const { return data_.GetSize(); }

    bool isDeleted(unsigned int i) const {
      if (i >= deleted_.size()) return false;
      return deleted_.at(i);
    }

    void Delete(unsigned int i) {
      deleted_[i] = true;
      nDeleted_++;
    }

    /**
     * will remove from storage the element i. This is a helper
     * function for SecondaryView.
     */
    void purge(unsigned int i) {
      unsigned int iStackBack = getSize() - 1;
      // search for last non-deleted particle on stack
      while (deleted_[iStackBack]) { iStackBack--; }
      // copy entry from iStackBack to iStackFront
      data_.Copy(iStackBack, i);
      if (deleted_[i]) nDeleted_--;
      deleted_[i] = deleted_[iStackBack];
      data_.DecrementSize();
      deleted_.pop_back();
    }

    /**
     * Function to perform eventual transformation from
     * StackIterator::GetIndex() to index in data stored in
     * TStackData data_. By default (and in almost all cases) this
     * should just be identiy. See class SecondaryView for an alternative implementation.
     */
    unsigned int GetIndexFromIterator(const unsigned int vI) const { return vI; }

    /**
     * @name Return reference to TStackData object data_ for data access
     * @{
     */
    StackDataValueType& GetStackData() { return data_; }
    const StackDataValueType& GetStackData() const { return data_; }
    ///@}
  };

} // namespace corsika::stack
