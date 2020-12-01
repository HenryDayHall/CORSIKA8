/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/logging/Logging.hpp>
#include <corsika/framework/stack/StackIteratorInterface.hpp>

#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>

namespace corsika {

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

  template <typename StackData, template <typename> typename MParticleInterface>
  class Stack {

    typedef typename std::remove_reference<StackData>::type value_type;

  public:
    typedef StackData stack_implementation_type; ///< this is the type of the
                                                 ///< user-provided data structure

    template <typename TSI>
    using pi_type = MParticleInterface<TSI>;

    /**
     * Via the StackIteratorInterface and ConstStackIteratorInterface
     * specialization, the type of the stack_iterator_type
     * template class is declared for a particular stack data
     * object. Using CRTP, this also determines the type of
     * MParticleInterface template class simultaneously.
     */
    typedef StackIteratorInterface<value_type, MParticleInterface, Stack>
        stack_iterator_type;

    typedef ConstStackIteratorInterface<value_type, MParticleInterface, Stack>
        const_stack_iterator_type;

    /**
     * this is the full type of the user-declared MParticleInterface
     */
    typedef typename stack_iterator_type::particle_interface_type particle_interface_type;
    /**
     * In all programming context, the object to access, copy, and
     * transport particle data is via the stack_iterator_type
     */
    typedef stack_iterator_type particle_type;

    //========================

    Stack() = default;

    Stack(Stack&) = delete; ///< since Stack can be very big, we don't want to copy it

    Stack& operator=(Stack&) =
        delete; ///< since Stack can be very big, we don't want to copy it

    /**
     * if StackData is a reference member we *HAVE* to initialize
     * it in the constructor, this is typically needed for SecondaryView
     */
    template <typename UType = StackData,
              typename = typename std::enable_if<std::is_reference<UType>::value>::type>
    Stack(StackData vD)
        : nDeleted_(0)
        , data_(vD)
        , deleted_(std::vector<bool>(data_.getSize(), false)) {}

    /**
     * This constructor takes any argument and passes it on to the
     * StackData user class. If the user did not provide a suited
     * constructor this will fail with an error message.
     *
     * Furthermore, this is disabled with enable_if for SecondaryView
     * stacks, where the inner data container is always a reference
     * and cannot be initialized here.
     */
    template <typename... TArgs, typename UType = StackData,
              typename = typename std::enable_if<std::is_reference<UType>::value>::type>
    Stack(TArgs... args)
        : nDeleted_(0)
        , data_(args...)
        , deleted_(std::vector<bool>(data_.getSize(), false)) {}

    /**
     * @name Most generic proxy methods for StackData data_
     * @{
     */
    unsigned int getCapacity() const { return data_.getCapacity(); }

    unsigned int getErased() const { return nDeleted_; }

    unsigned int getEntries() const { return getSize() - getErased(); }

    template <typename... TArgs>
    void clear(TArgs... args) {
      data_.clear(args...);
      deleted_ = std::vector<bool>(data_.getSize(), false);
      nDeleted_ = 0;
    }
    ///@}

    /**
     * @name These are functions required by std containers and std loops
     * @{
     */
    stack_iterator_type begin() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[i]) break;
      }
      return stack_iterator_type(*this, i);
    }

    stack_iterator_type end() { return stack_iterator_type(*this, getSize()); }

    stack_iterator_type last() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[getSize() - 1 - i]) break;
      }
      return stack_iterator_type(*this, getSize() - 1 - i);
    }

    const_stack_iterator_type begin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[i]) break;
      }
      return const_stack_iterator_type(*this, i);
    }

    const_stack_iterator_type end() const {
      return const_stack_iterator_type(*this, getSize());
    }

    const_stack_iterator_type last() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[getSize() - 1 - i]) break;
      }
      return const_stack_iterator_type(*this, getSize() - 1 - i);
    }

    const_stack_iterator_type cbegin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[i]) break;
      }
      return const_stack_iterator_type(*this, i);
    }

    const_stack_iterator_type cend() const {
      return const_stack_iterator_type(*this, getSize());
    }

    const_stack_iterator_type clast() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!deleted_[getSize() - 1 - i]) break;
      }
      return const_stack_iterator_type(*this, getSize() - 1 - i);
    }

    stack_iterator_type at(unsigned int i) { return stack_iterator_type(*this, i); }

    const_stack_iterator_type at(unsigned int i) const {
      return const_stack_iterator_type(*this, i);
    }

    stack_iterator_type first() { return stack_iterator_type{*this, 0}; }

    const_stack_iterator_type cfirst() const {
      return const_stack_iterator_type{*this, 0};
    }
    /// @}

    stack_iterator_type getNextParticle() {
      while (purgeLastIfDeleted()) {}
      return last();
    }

    /**
     * increase stack size, create new particle at end of stack
     */
    template <typename... TArgs>
    stack_iterator_type addParticle(const TArgs... v) {
      // C8LOG_TRACE("Stack::AddParticle");
      data_.incrementSize();
      deleted_.push_back(false);
      return stack_iterator_type(*this, getSize() - 1, v...);
    }

    void swap(stack_iterator_type a, stack_iterator_type b) {
      // C8LOG_TRACE("Stack::Swap");
      swap(a.getIndex(), b.getIndex());
    }

    void copy(stack_iterator_type a, stack_iterator_type b) {
      // C8LOG_TRACE("Stack::Copy");
      copy(a.getIndex(), b.getIndex());
    }

    void copy(const_stack_iterator_type a, stack_iterator_type b) {
      // C8LOG_TRACE("Stack::Copy");
      data_.copy(a.getIndex(), b.getIndex());
      if (deleted_[b.getIndex()] && !deleted_[a.getIndex()]) nDeleted_--;
      if (!deleted_[b.getIndex()] && deleted_[a.getIndex()]) nDeleted_++;
      deleted_[b.getIndex()] = deleted_[a.getIndex()];
    }

    void erase(stack_iterator_type p) {
      // C8LOG_TRACE("Stack::Delete");
      if (this->isEmpty()) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (deleted_[p.getIndex()]) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since already deleted");
      }
      this->erase(p.getIndex());
    }
    /**
     * delete this particle
     */
    void erase(particle_interface_type p) { this->erase(p.getIterator()); }

    /**
     * check if there are no further non-deleted particles on stack
     */
    bool isEmpty() { return getEntries() == 0; }

    /**
     * check if this particle was already deleted
     */
    bool isErased(const stack_iterator_type& p) const { return isErased(p.getIndex()); }

    bool isErased(const const_stack_iterator_type& p) const {
      return isErased(p.getIndex());
    }

    bool isErased(const particle_interface_type& p) const {
      return isErased(p.getIterator());
    }

    /**
     * Function to ultimatively remove the last entry from the stack,
     * if it was marked as deleted before. If this is not the case,
     * the function will just return false and do nothing.
     */
    bool purgeLastIfDeleted() {
      if (!deleted_.back())
        return false; // the last particle is not marked for deletion. Do nothing.
      // C8LOG_TRACE("Stack::purgeLastIfDeleted: yes");
      data_.decrementSize();
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
      for (unsigned int iDeleted = 0; iDeleted < getErased(); ++iDeleted) {
        // search first delete entry on stack
        while (!deleted_[iStackFront]) { iStackFront++; }
        // search for last non-deleted particle on stack
        while (deleted_[iStackBack]) { iStackBack--; }
        // copy entry from iStackBack to iStackFront
        data_.copy(iStackBack, iStackFront);
        data_.decrementSize();
      }
      deleted_.clear();
      nDeleted_ = 0;
    }

    unsigned int getSize() const { return data_.getSize(); }

    std::string as_string() const {
      std::string str(fmt::format("size {}, entries {}, deleted {} \n", getSize(),
                                  getEntries(), getErased()));
      // we make our own begin/end since we want ALL entries
      std::string new_line = "     ";
      for (unsigned int iPart = 0; iPart != getSize(); ++iPart) {
        const_stack_iterator_type itPart(*this, iPart);
        str += fmt::format("{}{}{}", new_line, itPart.as_string(),
                           (deleted_[itPart.getIndex()] ? " [deleted]" : ""));
        new_line = "\n     ";
      }
      return str;
    }

  protected:

    /**
     * increase stack size, create new particle at end of stack, related to parent
     * particle/projectile
     *
     * This should only get internally called from a
     * StackIterator::AddSecondary via ParticleBase
     */
    /*
    template <typename... TArgs>
    stack_iterator_type addSecondary(stack_iterator_type& parent, const TArgs... v) {
      CORSIKA_LOG_TRACE("Stack::AddSecondary");
      data_.incrementSize();
      deleted_.push_back(false);
      return stack_iterator_type(*this, getSize() - 1, parent, v...);
    }

    void swap(unsigned int a, unsigned int b) {
      CORSIKA_LOG_TRACE("Stack::Swap(unsigned int)");
      data_.swap(a, b);
      std::swap(deleted_[a], deleted_[b]);
    }
    void copy(unsigned int a, unsigned int b) {
      CORSIKA_LOG_TRACE("Stack::Copy");
      data_.copy(a, b);
      if (deleted_[b] && !deleted_[a]) nDeleted_--;
      if (!deleted_[b] && deleted_[a]) nDeleted_++;
      deleted_[b] = deleted_[a];
    }

    bool isDeleted(unsigned int i) const {
      if (i >= deleted_.size()) return false;
      return deleted_.at(i);
    }

    void erase(unsigned int i) {
      deleted_[i] = true;
      nDeleted_++;
    }
    */

    /*
     * will remove from storage the element i. This is a helper
     * function for SecondaryView.
     */

    /*
    void purge(unsigned int i) {
      unsigned int iStackBack = getSize() - 1;
      // search for last non-deleted particle on stack
      while (deleted_[iStackBack]) { iStackBack--; }
      // copy entry from iStackBack to iStackFront
      data_.copy(iStackBack, i);
      if (deleted_[i]) nDeleted_--;
      deleted_[i] = deleted_[iStackBack];
      data_.decrementSize();
      deleted_.pop_back();
    }
    */
    /**
     * increase stack size, create new particle at end of stack, related to parent
     * particle/projectile
     *
     * This should only get internally called from a
     * StackIterator::AddSecondary via ParticleBase
     */
    template <typename... TArgs>
    stack_iterator_type addSecondary(stack_iterator_type& parent, const TArgs... v) {
      // C8LOG_TRACE("Stack::AddSecondary");
      data_.incrementSize();
      deleted_.push_back(false);
      return stack_iterator_type(*this, getSize() - 1, parent, v...);
    }

    void swap(unsigned int const a, unsigned int const b) {
      // C8LOG_TRACE("Stack::Swap(unsigned int)");
      data_.swap(a, b);
      std::swap(deleted_[a], deleted_[b]);
    }
    void copy(unsigned int const a, unsigned int const b) {
      // C8LOG_TRACE("Stack::Copy");
      data_.copy(a, b);
      if (deleted_[b] && !deleted_[a]) nDeleted_--;
      if (!deleted_[b] && deleted_[a]) nDeleted_++;
      deleted_[b] = deleted_[a];
    }

    bool isErased(unsigned int const i) const {
      if (i >= deleted_.size()) return false;
      return deleted_.at(i);
    }

    void erase(unsigned int const i) {
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
      data_.copy(iStackBack, i);
      if (deleted_[i]) nDeleted_--;
      deleted_[i] = deleted_[iStackBack];
      data_.decrementSize();
      deleted_.pop_back();
    }

    /**
     * Function to perform eventual transformation from
     * StackIterator::getIndex() to index in data stored in
     * StackData data_. By default (and in almost all cases) this
     * should just be identiy. See class SecondaryView for an alternative implementation.
     */
    unsigned int getIndexFromIterator(const unsigned int vI) const {
      // this is too much: //C8LOG_TRACE("Stack::getIndexFromIterator({})={}", vI, vI);
      return vI;
    }

    /**
     * @name Return reference to StackData object data_ for data access
     * @{
     */
    value_type& getStackData() { return data_; }

    const value_type& getStackData() const { return data_; }
    ///@}
    ///

    ///
    friend class StackIteratorInterface<value_type, MParticleInterface, Stack>;
    friend class ConstStackIteratorInterface<value_type, MParticleInterface, Stack>;
    template <typename T1, //=StackData,
              template <typename>
              typename M1, //=MParticleInterface,
                           //             template<typename>typename M2>
              template <class T2, template <class> class T3> class MSecondaryProducer>
    friend class SecondaryView; //<StackData,MParticleInterface,M>; // access for
                                // SecondaryView

    friend class ParticleBase<stack_iterator_type>;

  protected:
    unsigned int nDeleted_ = 0;

  private:
    StackData data_; ///< this in general holds all the data and can be quite big
    std::vector<bool> deleted_; ///< bit field to flag deleted entries
  };

} // namespace corsika
