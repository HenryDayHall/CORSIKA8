/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/stack/Stack.h>

#include <stdexcept>
#include <vector>

namespace corsika::stack {

  /**
   * @class SecondaryView
   *
   * SecondaryView can only be constructed by giving a valid
   * Projectile particle, following calls to AddSecondary will
   * populate the original Stack, but will be directly accessible via
   * the SecondaryView, e.g.

     This allows to write code like
     \verbatim
     auto projectileInput = mainStack.GetNextParticle();
     const unsigned int nMain = mainStack.GetSize();
     SecondaryView<StackData, ParticleInterface> mainStackView(projectileInput);
     mainStackView.AddSecondary(...data...);
     mainStackView.AddSecondary(...data...);
     mainStackView.AddSecondary(...data...);
     mainStackView.AddSecondary(...data...);
     assert(mainStackView.GetSize() == 4);
     assert(mainStack.GetSize() = nMain+4);
     \endverbatim

     All operations possible on a Stack object are also possible on a
     SecondaryView object. This means you can add, delete, copy, swap,
     iterate, etc.

     *Further information about implementation (for developers):* All
     data is stored in the original stack privided at construction
     time. The secondary particle (view) indices are stored in an
     extra std::vector of SecondaryView class 'indices_' referring to
     the original stack slot indices. The index of the primary
     projectle particle is also explicitly stored in
     'projectile_index_'. StackIterator indices
     'i = StackIterator::GetIndex()' are referring to those numbers,
     where 'i==0' refers to the 'projectile_index_', and
     'StackIterator::GetIndex()>0' to 'indices_[i-1]', see function
     GetIndexFromIterator.
   */

  template <typename StackDataType, template <typename> typename ParticleInterface>
  class SecondaryView : public Stack<StackDataType&, ParticleInterface> {

    using ViewType = SecondaryView<StackDataType, ParticleInterface>;

  private:
    /**
     * Helper type for inside this class
     */
    using InnerStackTypeRef = Stack<StackDataType&, ParticleInterface>;
    using InnerStackTypeRef::getDeleted;

    /**
     * @name We need this "special" types with non-reference StackData for
     * the constructor of the SecondaryView class
     * @{
     */
    using InnerStackTypeValue = Stack<StackDataType, ParticleInterface>;

  public:
    using StackIteratorValue =
        StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                               ParticleInterface, InnerStackTypeValue>;
    using ConstStackIteratorValue =
        ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                    ParticleInterface, InnerStackTypeValue>;
    /// @}

    using StackIterator =
        StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                               ParticleInterface, ViewType>;
    using ConstStackIterator =
        ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                    ParticleInterface, ViewType>;

    /**
     * this is the full type of the declared ParticleInterface: typedef typename
     */
    using ParticleType = StackIterator;
    using ParticleInterfaceType = typename StackIterator::ParticleInterfaceType;

    friend class StackIteratorInterface<
        typename std::remove_reference<StackDataType>::type, ParticleInterface, ViewType>;

    friend class ConstStackIteratorInterface<
        typename std::remove_reference<StackDataType>::type, ParticleInterface, ViewType>;

  private:
    /**
     * This is not accessible, since we don't want to allow creating a
     * new stack.
     */
    template <typename... Args>
    SecondaryView(Args... args) = delete;

  private:
    InnerStackTypeValue& inner_stack_;
    unsigned int projectile_index_;
    std::vector<unsigned int> indices_;

  public:
    /**
       SecondaryView can only be constructed passing it a valid
       StackIterator to another Stack object
     **/
    SecondaryView(StackIteratorValue& vI)
        : Stack<StackDataType&, ParticleInterface>(vI.GetStackData())
        , inner_stack_(vI.GetStack())
        , projectile_index_(vI.GetIndex()) {}

    /**
     * This returns the projectile/parent in the original Stack, where this
     * SecondaryView is derived from. This projectile should not be
     * used to modify the Stack!
     */

    ConstStackIteratorValue parent() const {
      return ConstStackIteratorValue(inner_stack_, projectile_index_);
    }

    /**
     * This return a projectile of this SecondaryView, which can be
     * used to modify the SecondaryView
     */
    StackIterator GetProjectile() {
      // NOTE: 0 is special marker here for PROJECTILE, see GetIndexFromIterator
      return StackIterator(*this, 0);
    }

  public:
    template <typename... Args>
    StackIterator AddSecondary(const Args... v) {
      StackIterator proj = GetProjectile();
      return AddSecondary(proj, v...);
    }

    template <typename... Args>
    StackIterator AddSecondary(StackIterator& proj, const Args... v) {
      // make space on stack
      InnerStackTypeRef::GetStackData().IncrementSize();
      inner_stack_.deleted_.push_back(false);
      // get current number of secondaries on stack
      const unsigned int idSec = getSize();
      // determine index on (inner) stack where new particle will be located
      const unsigned int index = InnerStackTypeRef::GetStackData().GetSize() - 1;
      indices_.push_back(index);
      // NOTE: "+1" is since "0" is special marker here for PROJECTILE, see
      // GetIndexFromIterator
      return StackIterator(*this, idSec + 1, proj, v...);
    }

    /**
     * overwrite Stack::GetSize to return actual number of secondaries
     */
    unsigned int getSize() const { return indices_.size(); }
    unsigned int getEntries() const { return getSize() - getDeleted(); }
    bool IsEmpty() const { return getEntries() == 0; }

    /**
     * @name These are functions required by std containers and std loops
     * The Stack-versions must be overwritten, since here we need the correct
     * SecondaryView::getSize
     * @{
     */
    // NOTE: the "+1" is since "0" is special marker here for PROJECTILE, see
    // GetIndexFromIterator
    StackIterator begin() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(i)) break;
      }
      return StackIterator(*this, i + 1);
    }
    auto end() { return StackIterator(*this, getSize() + 1); }
    auto last() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(getSize() - 1 - i)) break;
      }
      return StackIterator(*this, getSize() - 1 - i + 1);
    }

    auto begin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(i)) break;
      }
      return ConstStackIterator(*this, i + 1);
    }
    auto end() const { return ConstStackIterator(*this, getSize() + 1); }
    auto last() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(getSize() - 1 - i)) break;
      }
      return ConstStackIterator(*this, getSize() - 1 - i + 1);
    }

    auto cbegin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(i)) break;
      }
      return ConstStackIterator(*this, i + 1);
    }
    auto cend() const { return ConstStackIterator(*this, getSize()); }
    auto clast() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(getSize() - 1 - i)) break;
      }
      return ConstStackIterator(*this, getSize() - 1 - i + 1);
    }
    /// @}

    /**
     * need overwrite Stack::Delete, since we want to call
     * SecondaryView::DeleteLast
     *
     * The particle is deleted on the underlying (internal) stack. The
     * local references in SecondaryView in indices_ must be fixed,
     * too.  The approach is to a) check if the particle 'p' is at the
     * very end of the internal stack, b) if not: move it there by
     * copying the last particle to the current particle location, c)
     * remove the last particle.
     *
     */
    void Delete(StackIterator p) {
      if (IsEmpty()) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (isDeleted(p.GetIndex() - 1)) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since already deleted");
      }
      inner_stack_.Delete(GetIndexFromIterator(p.GetIndex()));
      InnerStackTypeRef::nDeleted_++; // also count in SecondaryView
    }

    /**
     * need overwrite Stack::Delete, since we want to call SecondaryView::DeleteLast
     */
    // void Delete(ParticleInterfaceType p) { Delete(p.GetIterator()); }

    /**
     * return next particle from stack, need to overwrtie Stack::GetNextParticle to get
     * right reference
     */
    StackIterator GetNextParticle() {
      while (purgeLastIfDeleted()) {}
      return last();
    }

    /**
     * check if this particle was already deleted
     *
     * need to re-implement for SecondaryView since StackIterator types are a bit
     * different
     */
    bool isDeleted(const StackIterator& p) const { return isDeleted(p.GetIndex() - 1); }
    bool isDeleted(const ConstStackIterator& p) const {
      return isDeleted(p.GetIndex() - 1);
    }
    /**
     * delete this particle
     */
    bool isDeleted(const ParticleInterfaceType& p) const {
      return isDeleted(p.GetIterator());
    }

    /**
     * Function to ultimatively remove the last entry from the stack,
     * if it was marked as deleted before. If this is not the case,
     * the function will just return false and do nothing.
     */
    bool purgeLastIfDeleted() {
      if (!isDeleted(getSize() - 1))
        return false; // the last particle is not marked for deletion. Do nothing.
      inner_stack_.purge(GetIndexFromIterator(getSize()));
      InnerStackTypeRef::nDeleted_--;
      indices_.pop_back();
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
      unsigned int iStack = 0;
      unsigned int size = getSize();
      while (iStack < size) {
        if (isDeleted(iStack)) {
          inner_stack_.purge(iStack);
          indices_.erase(indices_.begin() + iStack);
        }
        size = getSize();
        iStack++;
      }
      InnerStackTypeRef::nDeleted_ = 0;
    }

  protected:
    // forward to inner stack
    // this also checks the allowed bounds of 'i'
    bool isDeleted(unsigned int i) const {
      if (i >= indices_.size()) return false;
      return inner_stack_.isDeleted(GetIndexFromIterator(i + 1));
    }

    /**
     * We only want to 'see' secondaries indexed in indices_. In this
     * function the conversion form iterator-index to stack-index is
     * performed.
     */
    unsigned int GetIndexFromIterator(const unsigned int vI) const {
      if (vI == 0) return projectile_index_;
      return indices_[vI - 1];
    }
  };

  /*
    See Issue 161

    unfortunately clang does not support this in the same way (yet) as
    gcc, so we have to distinguish here. If clang cataches up, we
    could remove the #if here and elsewhere. The gcc code is much more
    generic and universal.
  */
#if not defined(__clang__) && defined(__GNUC__) || defined(__GNUG__)
  template <typename S, template <typename> typename _PIType = S::template PIType>
  struct MakeView {
    using type = corsika::stack::SecondaryView<typename S::StackImpl, _PIType>;
  };
#endif

} // namespace corsika::stack
