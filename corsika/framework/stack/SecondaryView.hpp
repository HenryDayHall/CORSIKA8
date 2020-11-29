/*
 * (c) copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/framework/stack/Stack.hpp>

//#include <corsika/logging/Logging.h>

#include <stdexcept>
#include <vector>

namespace corsika {

  // forward-decl:
  template <class T1, template <class> class T2>
  class DefaultSecondaryProducer;

  /**
   * @class SecondaryView
   *
   * SecondaryView can only be constructed by giving a valid
   * Projectile particle, following calls to addSecondary will
   * populate the original Stack, but will be directly accessible via
   * the SecondaryView, e.g.

     This allows to write code like
     \verbatim
     auto projectileInput = mainStack.getNextParticle();
     const unsigned int nMain = mainStack.getSize();
     SecondaryView<StackData, ParticleInterface> mainStackView(projectileInput);
     mainStackView.addSecondary(...data...);
     mainStackView.addSecondary(...data...);
     mainStackView.addSecondary(...data...);
     mainStackView.addSecondary(...data...);
     assert(mainStackView.getSize() == 4);
     assert(mainStack.getSize() = nMain+4);
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
     'i = StackIterator::getIndex()' are referring to those numbers,
     where 'i==0' refers to the 'projectile_index_', and
     'StackIterator::getIndex()>0' to 'indices_[i-1]', see function
     getIndexFromIterator.
   */

  template <typename StackDataType,
            template <typename> typename ParticleInterface,
            template <typename T1, template <class> class T2> class MSecondaryProducer = DefaultSecondaryProducer>
  class SecondaryView : public Stack<StackDataType&, ParticleInterface>,
                        public MSecondaryProducer<StackDataType, ParticleInterface>
  {

	//using ViewType = SecondaryView<StackDataType, ParticleInterface, MSecondaryProducer>;
    typedef SecondaryView<StackDataType, ParticleInterface, MSecondaryProducer> view_type;
    /**
     * Helper type for inside this class
     */
    //using InnerStackTypeRef = Stack<StackDataType&, ParticleInterface>;
    typedef Stack<StackDataType&, ParticleInterface> inner_stack_reference_type;

    //using InnerStackTypeValue = Stack<StackDataType, ParticleInterface>;
    typedef Stack<StackDataType, ParticleInterface>  inner_stack_value_type;

    using inner_stack_reference_type::getDeleted;

    /**
     * @name We need this "special" types with non-reference StackData for
     * the constructor of the SecondaryView class
     * @{
     */


  public:

//    using StackIteratorValue =
//        StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
//                               ParticleInterface, InnerStackTypeValue>;
   typedef  StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                   ParticleInterface, inner_stack_value_type> stack_value_iterator;

//    using ConstStackIteratorValue =
//        ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
//                                    ParticleInterface, inner_stack_value_type>;

   typedef ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                       ParticleInterface, inner_stack_value_type> const_stack_value_iterator;
       /// @}

//    using StackIterator =
//        StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
//                               ParticleInterface, view_type>;

    typedef  StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                               ParticleInterface, view_type> stack_view_iterator;

//    using ConstStackIterator =
//        ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
//                                    ParticleInterface, view_type>;

    typedef ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                    ParticleInterface, view_type> const_stack_view_iterator;

    /**
     * this is the full type of the declared ParticleInterface: typedef typename
     */
    using ParticleType = StackIterator;
    using ParticleInterfaceType = typename StackIterator::ParticleInterfaceType;

    friend class StackIteratorInterface<
        typename std::remove_reference<StackDataType>::type, ParticleInterface, view_type>;

    friend class ConstStackIteratorInterface<
        typename std::remove_reference<StackDataType>::type, ParticleInterface, view_type>;

    friend class ParticleBase<StackIterator>;


    /**
     * This is not accessible, since we don't want to allow creating a
     * new stack.
     */
    template <typename... Args>
    SecondaryView(Args... args) = delete;
    SecondaryView() = delete;

   /**
       SecondaryView can only be constructed passing it a valid
       StackIterator to another Stack object (here: lvalue)
     **/
    SecondaryView(stack_value_iterator& particle)
        : Stack<StackDataType&, ParticleInterface>(particle.getStackData())
        , MSecondaryProducer<StackDataType, ParticleInterface>{particle}
        , inner_stack_(particle.getStack())
        , projectile_index_(particle.getIndex()) {
      C8LOG_TRACE("SecondaryView::SecondaryView(particle&)");
    }
    /**
       SecondaryView can only be constructed passing it a valid
       StackIterator to another Stack object (here: rvalue)
     **/
    SecondaryView(stack_value_iterator&& particle)
        : Stack<StackDataType&, ParticleInterface>(particle.getStackData())
        , MSecondaryProducer<StackDataType, ParticleInterface>{particle}
        , inner_stack_(particle.getStack())
        , projectile_index_(particle.getIndex()) {
      C8LOG_TRACE("SecondaryView::SecondaryView(particle&&)");
    }
    /**
     * Also allow to create a new View from a Projectile (StackIterator on View)
     *
     * Note, the view generated this way will be equivalent to the orignal view in
     * terms of reference to the underlying data stack. It is not a "view to a view".
     */
    SecondaryView(view_type& view, stack_view_iterator& projectile)
        : Stack<StackDataType&, ParticleInterface>{view.getStackData()}
        , MSecondaryProducer<StackDataType, ParticleInterface>{stack_value_iterator{
              view.inner_stack_, view.getIndexFromIterator(projectile.getIndex())}}
        , inner_stack_{view.inner_stack_}
        , projectile_index_{view.getIndexFromIterator(projectile.getIndex())} {
      C8LOG_TRACE("SecondaryView::SecondaryView(view, projectile)");
    }

    /**
     * This returns the projectile/parent in the original Stack, where this
     * SecondaryView is derived from. This projectile should not be
     * used to modify the Stack!
     */
    stack_value_iterator parent()
        const { // todo: check if this can't be Conststack_value_iterator
      return stack_value_iterator(inner_stack_, projectile_index_);
    }

    /**
     * This returns the projectile/parent in the original Stack, where this
     * SecondaryView is derived from. This projectile should not be
     * used to modify the Stack!
     */
    stack_value_iterator asNewParent() const {
      return stack_value_iterator(inner_stack_, projectile_index_);
    }

    /**
     * This return a projectile of this SecondaryView, which can be
     * used to modify the SecondaryView
     */
    stack_view_iterator getProjectile() {
      // NOTE: 0 is special marker here for PROJECTILE, see getIndexFromIterator
      return stack_view_iterator(*this, 0);
    }
    /**
     * Method to add a new secondary particle on this SecondaryView
     */
    template <typename... Args>
    stack_view_iterator addSecondary(const Args... v) {
      C8LOG_TRACE("SecondaryView::addSecondary(Args&&)");
      stack_view_iterator proj = getProjectile(); // make this const
      return addSecondary(proj, v...);
    }
    /**
     * overwrite Stack::getSize to return actual number of secondaries
     */
    unsigned int getSize() const {
    	return indices_.size();
    }

    unsigned int getEntries() const {
    	return getSize() - getDeleted();
    }

    bool IsEmpty() const {
    	return getEntries() == 0;
    }

    /**
     * @name These are functions required by std containers and std loops
     * The Stack-versions must be overwritten, since here we need the correct
     * SecondaryView::getSize
     * @{
     */
    // NOTE: the "+1" is since "0" is special marker here for PROJECTILE, see
    // getIndexFromIterator
    stack_view_iterator begin() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(i)) break;
      }
      return stack_view_iterator(*this, i + 1);
    }

    auto end() {
    	return stack_view_iterator(*this, getSize() + 1);
    }

    auto last() {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(getSize() - 1 - i)) break;
      }
      return stack_view_iterator(*this, getSize() - 1 - i + 1);
    }

    auto begin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(i)) break;
      }

      return const_stack_view_iterator(*this, i + 1);
    }

    auto end() const {
    	return const_stack_view_iterator(*this, getSize() + 1);
    }

    auto last() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(getSize() - 1 - i)) break;
      }
      return const_stack_view_iterator(*this, getSize() - 1 - i + 1);
    }

    auto cbegin() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(i)) break;
      }
      return const_stack_view_iterator(*this, i + 1);
    }

    auto cend() const {
    	return const_stack_view_iterator(*this, getSize());
    }

    auto clast() const {
      unsigned int i = 0;
      for (; i < getSize(); ++i) {
        if (!isDeleted(getSize() - 1 - i)) break;
      }
      return const_stack_view_iterator(*this, getSize() - 1 - i + 1);
    }

    stack_view_iterator at(unsigned int i) {
    	return stack_view_iterator(*this, i);
    }

    const_stack_view_iterator at(unsigned int i) const {
    	return const_stack_view_iterator(*this, i);
    }

    stack_view_iterator first() {
    	return stack_view_iterator{*this, 0};
    }

    const_stack_view_iterator cfirst() const {
    	return const_stack_view_iterator{*this, 0};
    }
    /// @}

    void swap(stack_view_iterator a, stack_view_iterator b) {
      C8LOG_TRACE("View::swap");
      inner_stack_.swap(getIndexFromIterator(a.getIndex()),
                        getIndexFromIterator(b.getIndex()));
    }
    void copy(stack_view_iterator a, stack_view_iterator b) {
      C8LOG_TRACE("View::copy");
      inner_stack_.copy(getIndexFromIterator(a.getIndex()),
                        getIndexFromIterator(b.getIndex()));
    }
    void copy(const_stack_view_iterator a, stack_view_iterator b) {
      C8LOG_TRACE("View::copy");
      inner_stack_.copy(getIndexFromIterator(a.getIndex()),
                        getIndexFromIterator(b.getIndex()));
    }

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
    void erase(stack_view_iterator p) {
      C8LOG_TRACE("SecondaryView::Delete");
      if (IsEmpty()) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (isDeleted(p.getIndex() - 1)) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since already deleted");
      }
      inner_stack_.Delete(getIndexFromIterator(p.getIndex()));
      inner_stack_reference_type::nDeleted_++; // also count in SecondaryView
    }

    /**
     * return next particle from stack, need to overwrtie Stack::getNextParticle to get
     * right reference
     */
    stack_view_iterator getNextParticle() {
      while (purgeLastIfDeleted()) {}
      return last();
    }

    /**
     * check if this particle was already deleted
     *
     * need to re-implement for SecondaryView since StackIterator types are a bit
     * different
     */
    bool isDeleted(const stack_view_iterator& p) const {
    	return isDeleted(p.getIndex() - 1);
    }

    bool isDeleted(const const_stack_view_iterator& p) const {
      return isDeleted(p.getIndex() - 1);
    }
    /**
     * delete this particle
     */
    bool isDeleted(const ParticleInterfaceType& p) const {
      return isDeleted(p.getIterator());
    }

    /**
     * Function to ultimatively remove the last entry from the stack,
     * if it was marked as deleted before. If this is not the case,
     * the function will just return false and do nothing.
     */
    bool purgeLastIfDeleted() {
      C8LOG_TRACE("SecondaryView::purgeLastIfDeleted");
      if (!isDeleted(getSize() - 1))
        return false; // the last particle is not marked for deletion. Do nothing.
      inner_stack_.purge(getIndexFromIterator(getSize()));
      inner_stack_reference_type::nDeleted_--;
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
      inner_stack_reference_type::nDeleted_ = 0;
    }

    std::string as_string() const {
      std::string str(fmt::format("size {}\n", getSize()));
      // we make our own begin/end since we want ALL entries
      std::string new_line = "     ";
      for (unsigned int iPart = 0; iPart != getSize(); ++iPart) {
    	  const_stack_view_iterator itPart(*this, iPart);
        str += fmt::format(
            "{}{}{}", new_line, itPart.as_string(),
            (inner_stack_.deleted_[getIndexFromIterator(itPart.getIndex())] ? " [deleted]"
                                                                            : ""));
        new_line = "\n     ";
      }
      return str;
    }

  protected:

     /**
      * Overwrite of Stack::StackIterator
      *
      * increase stack size, create new particle at end of stack,
      * related to parent particle/projectile
      *
      * This should only get internally called from a
      * StackIterator::addSecondary via ParticleBase
      */
     template <typename... Args>
     stack_view_iterator addSecondary(stack_view_iterator& proj, const Args... v) {
       C8LOG_TRACE("SecondaryView::addSecondary(StackIterator&, Args&&)");
       // make space on stack
       inner_stack_reference_type::getStackData().IncrementSize();
       inner_stack_.deleted_.push_back(false);
       // get current number of secondaries on stack
       const unsigned int idSec = getSize();
       // determine index on (inner) stack where new particle will be located
       const unsigned int index = inner_stack_reference_type::getStackData().getSize() - 1;
       indices_.push_back(index);
       // NOTE: "+1" is since "0" is special marker here for PROJECTILE, see
       // getIndexFromIterator
       auto sec = stack_view_iterator(*this, idSec + 1, proj, v...);
       MSecondaryProducer<StackDataType, ParticleInterface>::new_secondary(sec);
       return sec;
     }

    // forward to inner stack
    // this also checks the allowed bounds of 'i'
    bool isDeleted(unsigned int i) const {
      if (i >= indices_.size()) return false;
      return inner_stack_.isDeleted(getIndexFromIterator(i + 1));
    }

    /**
     * We only want to 'see' secondaries indexed in indices_. In this
     * function the conversion form iterator-index to stack-index is
     * performed.
     */
    unsigned int getIndexFromIterator(const unsigned int vI) const {
      // this is too much: C8LOG_TRACE("SecondaryView::getIndexFromIterator({})={}", vI,
      // (vI?indices_[vI-1]:projectile_index_));
      if (vI == 0) return projectile_index_;
      return indices_[vI - 1];
    }

  private:
    inner_stack_value_type& inner_stack_;
    unsigned int projectile_index_;
    std::vector<unsigned int> indices_;
  };



  /**
   * Class to handle the generation of new secondaries. Used as default mix-in for
   * SecondaryView.
   */
  template <class T1, template <class> class T2>
  class DefaultSecondaryProducer {
    using View = SecondaryView<T1, T2, DefaultSecondaryProducer>;

  public:
    static bool constexpr has_event{false};

    /**
     * Method is called after a new secondary has been created on the
     * SecondaryView. Extra logic can be introduced here.
     *
     * The input Particle is the new secondary that was produced and
     * is of course a reference into the SecondaryView itself.
     */
    template <typename Particle>
    auto new_secondary(Particle&&) const {
      C8LOG_TRACE("DefaultSecondaryProducer::new_secondary(Particle&&)");
    }

    /**
     * Method is called when a new SecondaryView is being created
     * created. Extra logic can be introduced here.
     *
     * The input Particle is a reference object into the original
     * parent stack! It is not a reference into the SecondaryView
     * itself.
     */
    template <typename Particle>
    DefaultSecondaryProducer(Particle const&) {
      C8LOG_TRACE("DefaultSecondaryProducer::DefaultSecondaryProducer(Particle&)");
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
  template <typename TStack,
            template <class TStack_, template <class> class MPIType_>
            class MSecondaryProducer = corsika::stack::DefaultSecondaryProducer,
            template <typename> typename MPIType_ = TStack::template MPIType>
  struct MakeView {
    using type = corsika::stack::SecondaryView<typename TStack::StackImpl, MPIType_, MSecondaryProducer>;
  };
#endif

} // namespace corsika


#include <corsika/detail/framework/stack/SecondaryView.inl>
