/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/stack/StackIteratorInterface.hpp>

#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>

namespace corsika {

  template <typename StackData, template <typename> typename MParticleInterface>
  template <typename... TArgs>
  void Stack<StackData, MParticleInterface>::clear(TArgs... args) {
    data_.clear(args...);
    deleted_ = std::vector<bool>(data_.getSize(), false);
    nDeleted_ = 0;
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::begin() {
    unsigned int i = 0;
    for (; i < getSize(); ++i) {
      if (!deleted_[i]) break;
    }
    return stack_iterator_type(*this, i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::end() {
    return stack_iterator_type(*this, getSize());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::last() {
    unsigned int i = 0;
    for (; i < getSize(); ++i) {
      if (!deleted_[getSize() - 1 - i]) break;
    }
    return stack_iterator_type(*this, getSize() - 1 - i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::begin() const {
    unsigned int i = 0;
    for (; i < getSize(); ++i) {
      if (!deleted_[i]) break;
    }
    return const_stack_iterator_type(*this, i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::end() const {
    return const_stack_iterator_type(*this, getSize());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::last() const {
    unsigned int i = 0;
    for (; i < getSize(); ++i) {
      if (!deleted_[getSize() - 1 - i]) break;
    }
    return const_stack_iterator_type(*this, getSize() - 1 - i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::cbegin() const {
    unsigned int i = 0;
    for (; i < getSize(); ++i) {
      if (!deleted_[i]) break;
    }
    return const_stack_iterator_type(*this, i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::cend() const {
    return const_stack_iterator_type(*this, getSize());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::clast() const {
    unsigned int i = 0;
    for (; i < getSize(); ++i) {
      if (!deleted_[getSize() - 1 - i]) break;
    }

    return const_stack_iterator_type(*this, getSize() - 1 - i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::at(unsigned int i) {
    return stack_iterator_type(*this, i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::at(unsigned int i) const {
    return const_stack_iterator_type(*this, i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::first() {
    return stack_iterator_type{*this, 0};
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::const_stack_iterator_type
  Stack<StackData, MParticleInterface>::cfirst() const {
    return const_stack_iterator_type{*this, 0};
  }
  /// @}

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::getNextParticle() {
    while (purgeLastIfDeleted()) {}
    return last();
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  template <typename... TArgs>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::addParticle(const TArgs... v) {
    CORSIKA_LOG_TRACE("Stack::AddParticle");
    data_.incrementSize();
    deleted_.push_back(false);
    return stack_iterator_type(*this, getSize() - 1, v...);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::swap(stack_iterator_type a,
                                                  stack_iterator_type b) {
    CORSIKA_LOG_TRACE("Stack::Swap");
    swap(a.getIndex(), b.getIndex());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::copy(stack_iterator_type a,
                                                  stack_iterator_type b) {
    CORSIKA_LOG_TRACE("Stack::Copy");
    copy(a.getIndex(), b.getIndex());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::copy(const_stack_iterator_type a,
                                                  stack_iterator_type b) {
    CORSIKA_LOG_TRACE("Stack::Copy");
    data_.copy(a.getIndex(), b.getIndex());
    if (deleted_[b.getIndex()] && !deleted_[a.getIndex()]) nDeleted_--;
    if (!deleted_[b.getIndex()] && deleted_[a.getIndex()]) nDeleted_++;
    deleted_[b.getIndex()] = deleted_[a.getIndex()];
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::erase(stack_iterator_type p) {
    CORSIKA_LOG_TRACE("Stack::Delete");
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

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::erase(particle_interface_type p) {
    this->erase(p.getIterator());
  }

  /**
   * check if there are no further non-deleted particles on stack
   */

  template <typename StackData, template <typename> typename MParticleInterface>
  bool Stack<StackData, MParticleInterface>::isEmpty() {
    return getEntries() == 0;
  }

  /**
   * check if this particle was already deleted
   */

  template <typename StackData, template <typename> typename MParticleInterface>
  bool Stack<StackData, MParticleInterface>::isErased(
      const stack_iterator_type& p) const {
    return isErased(p.getIndex());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  bool Stack<StackData, MParticleInterface>::isErased(
      const const_stack_iterator_type& p) const {
    return isErased(p.getIndex());
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  bool Stack<StackData, MParticleInterface>::isErased(
      const particle_interface_type& p) const {
    return isErased(p.getIterator());
  }

  /**
   * Function to ultimatively remove the last entry from the stack,
   * if it was marked as deleted before. If this is not the case,
   * the function will just return false and do nothing.
   */

  template <typename StackData, template <typename> typename MParticleInterface>
  bool Stack<StackData, MParticleInterface>::purgeLastIfDeleted() {
    if (!deleted_.back())
      return false; // the last particle is not marked for deletion. Do nothing.

    CORSIKA_LOG_TRACE("Stack::purgeLastIfDeleted: yes");
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

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::purge() {
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

  template <typename StackData, template <typename> typename MParticleInterface>
  unsigned int Stack<StackData, MParticleInterface>::getSize() const {
    return data_.getSize();
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  std::string Stack<StackData, MParticleInterface>::asString() const {
    std::string str(fmt::format("size {}, entries {}, deleted {} \n", getSize(),
                                getEntries(), getErased()));
    // we make our own begin/end since we want ALL entries
    std::string new_line = "     ";
    for (unsigned int iPart = 0; iPart != getSize(); ++iPart) {
      const_stack_iterator_type itPart(*this, iPart);
      str += fmt::format("{}{}{}", new_line, itPart.asString(),
                         (deleted_[itPart.getIndex()] ? " [deleted]" : ""));
      new_line = "\n     ";
    }
    return str;
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  template <typename... TArgs>
  typename Stack<StackData, MParticleInterface>::stack_iterator_type
  Stack<StackData, MParticleInterface>::addSecondary(stack_iterator_type& parent,
                                                     const TArgs... v) {
    CORSIKA_LOG_TRACE("Stack::AddSecondary");
    data_.incrementSize();
    deleted_.push_back(false);
    return stack_iterator_type(*this, getSize() - 1, parent, v...);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::swap(unsigned int const a,
                                                  unsigned int const b) {
    CORSIKA_LOG_TRACE("Stack::Swap(unsigned int)");
    data_.swap(a, b);
    std::swap(deleted_[a], deleted_[b]);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::copy(unsigned int const a,
                                                  unsigned int const b) {
    CORSIKA_LOG_TRACE("Stack::Copy");
    data_.copy(a, b);
    if (deleted_[b] && !deleted_[a]) nDeleted_--;
    if (!deleted_[b] && deleted_[a]) nDeleted_++;
    deleted_[b] = deleted_[a];
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  bool Stack<StackData, MParticleInterface>::isErased(unsigned int const i) const {
    if (i >= deleted_.size()) return false;
    return deleted_.at(i);
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::erase(unsigned int const i) {
    deleted_[i] = true;
    nDeleted_++;
  }

  /**
   * will remove from storage the element i. This is a helper
   * function for SecondaryView.
   */

  template <typename StackData, template <typename> typename MParticleInterface>
  void Stack<StackData, MParticleInterface>::purge(unsigned int i) {
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

  template <typename StackData, template <typename> typename MParticleInterface>
  unsigned int Stack<StackData, MParticleInterface>::getIndexFromIterator(
      const unsigned int vI) const {
    // this is too much: CORSIKA_LOG_TRACE("Stack::getIndexFromIterator({})={}", vI, vI);
    return vI;
  }

  /**
   * @name Return reference to StackData object data_ for data access
   * @{
   */

  template <typename StackData, template <typename> typename MParticleInterface>
  typename Stack<StackData, MParticleInterface>::value_type&
  Stack<StackData, MParticleInterface>::getStackData() {
    return data_;
  }

  template <typename StackData, template <typename> typename MParticleInterface>
  const typename Stack<StackData, MParticleInterface>::value_type&
  Stack<StackData, MParticleInterface>::getStackData() const {
    return data_;
  }

} // namespace corsika
