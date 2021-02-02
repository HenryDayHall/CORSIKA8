/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/stack/Stack.hpp>
#include <corsika/framework/core/Logging.hpp>

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

  template <typename TStackDataType, template <typename> typename TParticleInterface,
            template <typename T1, template <class> class T2> class MSecondaryProducer =
                DefaultSecondaryProducer>
  class SecondaryView : public Stack<TStackDataType&, TParticleInterface>,
                        public MSecondaryProducer<TStackDataType, TParticleInterface> {

    // using ViewType = SecondaryView<TStackDataType, TParticleInterface,
    // MSecondaryProducer>;
    typedef SecondaryView<TStackDataType, TParticleInterface, MSecondaryProducer>
        view_type;
    /**
     * Helper type for inside this class
     */
    // using InnerStackTypeRef = Stack<TStackDataType&, TParticleInterface>;
    typedef Stack<TStackDataType&, TParticleInterface> inner_stack_reference_type;

    /**
     * @name We need this "value" types with non-reference TStackData for
     * the constructor of the SecondaryView class
     * @{
     */
    // using InnerStackTypeValue = Stack<TStackDataType, TParticleInterface>;
    typedef Stack<TStackDataType, TParticleInterface> inner_stack_value_type;

  public:
    typedef StackIteratorInterface<typename std::remove_reference<TStackDataType>::type,
                                   TParticleInterface, inner_stack_value_type>
        stack_value_iterator;

    //    using ConstStackIteratorValue =
    //        ConstStackIteratorInterface<typename
    //        std::remove_reference<TStackDataType>::type,
    //                                    TParticleInterface, inner_stack_value_type>;

    typedef ConstStackIteratorInterface<
        typename std::remove_reference<TStackDataType>::type, TParticleInterface,
        inner_stack_value_type>
        const_stack_value_iterator;
    /// @}

    //    using StackIterator =
    //        StackIteratorInterface<typename std::remove_reference<TStackDataType>::type,
    //                               TParticleInterface, view_type>;

    typedef StackIteratorInterface<typename std::remove_reference<TStackDataType>::type,
                                   TParticleInterface, view_type>
        stack_view_iterator;

    typedef ConstStackIteratorInterface<
        typename std::remove_reference<TStackDataType>::type, TParticleInterface,
        view_type>
        const_stack_view_iterator;

    /**
     * this is the full type of the declared TParticleInterface:
     */
    using ParticleType = stack_view_iterator;
    using ParticleInterfaceType = typename stack_view_iterator::particle_interface_type;

    /**
     * This is not accessible, since we don't want to allow creating a
     * new stack.
     */
    template <typename... TArgs>
    SecondaryView(TArgs... args) = delete;
    SecondaryView() = delete;

    /**
        SecondaryView can only be constructed passing it a valid
        stack_view_iterator to another Stack object (here: lvalue)
      **/
    SecondaryView(stack_value_iterator& particle)
        : Stack<TStackDataType&, TParticleInterface>(particle.getStackData())
        , MSecondaryProducer<TStackDataType, TParticleInterface>{particle}
        , inner_stack_(particle.getStack())
        , projectile_index_(particle.getIndex()) {
      CORSIKA_LOG_TRACE("SecondaryView::SecondaryView(particle&)");
    }
    /**
       SecondaryView can only be constructed passing it a valid
       stack_view_iterator to another Stack object (here: rvalue)
     **/
    SecondaryView(stack_value_iterator&& particle)
        : Stack<TStackDataType&, TParticleInterface>(particle.getStackData())
        , MSecondaryProducer<TStackDataType, TParticleInterface>{particle}
        , inner_stack_(particle.getStack())
        , projectile_index_(particle.getIndex()) {
      CORSIKA_LOG_TRACE("SecondaryView::SecondaryView(particle&&)");
    }
    /**
     * Also allow to create a new View from a Projectile (stack_view_iterator on View)
     *
     * Note, the view generated this way will be equivalent to the orignal view in
     * terms of reference to the underlying data stack. It is not a "view to a view".
     */
    SecondaryView(view_type& view, stack_view_iterator& projectile)
        : Stack<TStackDataType&, TParticleInterface>{view.getStackData()}
        , MSecondaryProducer<TStackDataType, TParticleInterface>{stack_value_iterator{
              view.inner_stack_, view.getIndexFromIterator(projectile.getIndex())}}
        , inner_stack_{view.inner_stack_}
        , projectile_index_{view.getIndexFromIterator(projectile.getIndex())} {
      CORSIKA_LOG_TRACE("SecondaryView::SecondaryView(view, projectile)");
    }

    /**
     * This returns the projectile/parent in the original Stack, where this
     * SecondaryView is derived from. This projectile should not be
     * used to modify the Stack!
     */
    inline stack_value_iterator parent()
        const { // todo: check if this can't be Conststack_value_iterator
      return stack_value_iterator(inner_stack_, projectile_index_);
    }

    /**
     * This returns the projectile/parent in the original Stack, where this
     * SecondaryView is derived from. This projectile should not be
     * used to modify the Stack!
     */
    inline stack_value_iterator asNewParent() const {
      return stack_value_iterator(inner_stack_, projectile_index_);
    }

    /**
     * This return a projectile of this SecondaryView, which can be
     * used to modify the SecondaryView
     */
    inline stack_view_iterator getProjectile() {
      // NOTE: 0 is special marker here for PROJECTILE, see getIndexFromIterator
      return stack_view_iterator(*this, 0);
    }

    /**
     * This return a projectile of this SecondaryView, which can be
     * used to modify the SecondaryView
     */
    inline const_stack_view_iterator getProjectile() const {
      // NOTE: 0 is special marker here for PROJECTILE, see getIndexFromIterator
      return const_stack_view_iterator(*this, 0);
    }

    /**
     * Method to add a new secondary particle on this SecondaryView
     */
    template <typename... Args>
    inline stack_view_iterator addSecondary(const Args... v);

    /**
     * overwrite Stack::getSize to return actual number of secondaries
     */
    inline unsigned int getSize() const { return indices_.size(); }

    inline unsigned int getEntries() const {
      return getSize() - inner_stack_reference_type::getErased();
    }

    inline bool isEmpty() const { return getEntries() == 0; }

    /**
     * @name These are functions required by std containers and std loops
     * The Stack-versions must be overwritten, since here we need the correct
     * SecondaryView::getSize
     * @{
     */
    // NOTE: the "+1" is since "0" is special marker here for PROJECTILE, see
    // getIndexFromIterator
    inline stack_view_iterator begin();

    inline stack_view_iterator end() { return stack_view_iterator(*this, getSize() + 1); }

    inline stack_view_iterator last();

    inline const_stack_view_iterator begin() const;

    inline const_stack_view_iterator end() const {
      return const_stack_view_iterator(*this, getSize() + 1);
    }

    inline const_stack_view_iterator last() const;

    inline const_stack_view_iterator cbegin() const;

    inline const_stack_view_iterator cend() const {
      return const_stack_view_iterator(*this, getSize());
    }

    inline const_stack_view_iterator clast() const;

    inline stack_view_iterator at(unsigned int i) {
      return stack_view_iterator(*this, i);
    }

    inline const_stack_view_iterator at(unsigned int i) const {
      return const_stack_view_iterator(*this, i);
    }

    inline stack_view_iterator first() { return stack_view_iterator{*this, 0}; }

    inline const_stack_view_iterator cfirst() const {
      return const_stack_view_iterator{*this, 0};
    }
    /// @}

    inline void swap(stack_view_iterator a, stack_view_iterator b);

    inline void copy(stack_view_iterator a, stack_view_iterator b);

    inline void copy(const_stack_view_iterator a, stack_view_iterator b);

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
    inline void erase(stack_view_iterator p);

    /**
     * return next particle from stack, need to overwrtie Stack::getNextParticle to get
     * right reference
     */
    inline stack_view_iterator getNextParticle() {
      while (purgeLastIfDeleted()) {}
      return last();
    }

    /**
     * check if this particle was already deleted
     *
     * need to re-implement for SecondaryView since stack_view_iterator types are a bit
     * different
     */
    inline bool isErased(const stack_view_iterator& p) const {
      return isErased(p.getIndex() - 1);
    }

    inline bool isErased(const const_stack_view_iterator& p) const {
      return isErased(p.getIndex() - 1);
    }
    /**
     * delete this particle
     */
    inline bool isErased(const ParticleInterfaceType& p) const {
      return isErased(p.getIterator());
    }

    inline bool isDeleted(const const_stack_view_iterator& p) const {
      return isDeleted(p.getIndex() - 1);
    }
    /**
     * Function to ultimatively remove the last entry from the stack,
     * if it was marked as deleted before. If this is not the case,
     * the function will just return false and do nothing.
     */
    inline bool purgeLastIfDeleted();

    /**
     * Function to ultimatively remove all entries from the stack
     * marked as deleted.
     *
     * Careful: this will re-order the entries on the stack, since
     * "gaps" in the stack are filled with entries from the back
     * (copied).
     */
    inline void purge();

    inline std::string asString() const;

  protected:
    friend class StackIteratorInterface<
        typename std::remove_reference<TStackDataType>::type, TParticleInterface,
        view_type>;

    friend class ConstStackIteratorInterface<
        typename std::remove_reference<TStackDataType>::type, TParticleInterface,
        view_type>;

    friend class ParticleBase<stack_view_iterator>;

    /**
     * Overwrite of Stack::stack_view_iterator
     *
     * increase stack size, create new particle at end of stack,
     * related to parent particle/projectile
     *
     * This should only get internally called from a
     * stack_view_iterator::addSecondary via ParticleBase
     */
    template <typename... Args>
    inline stack_view_iterator addSecondary(stack_view_iterator& proj, const Args... v);

    // forward to inner stack
    // this also checks the allowed bounds of 'i'
    inline bool isErased(unsigned int i) const {
      if (i >= indices_.size()) return false;
      return inner_stack_.isErased(getIndexFromIterator(i + 1));
    }

    /**
     * We only want to 'see' secondaries indexed in indices_. In this
     * function the conversion form iterator-index to stack-index is
     * performed.
     */
    inline unsigned int getIndexFromIterator(const unsigned int vI) const {
      // this is too much: CORSIKA_LOG_TRACE("SecondaryView::getIndexFromIterator({})={}",
      // vI, (vI?indices_[vI-1]:projectile_index_));
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
    inline void new_secondary(Particle&&) const {
      CORSIKA_LOG_TRACE("DefaultSecondaryProducer::new_secondary(Particle&&)");
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
    inline DefaultSecondaryProducer(Particle const&) {

      CORSIKA_LOG_TRACE("DefaultSecondaryProducer::DefaultSecondaryProducer(Particle&)");
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
            template <class TStack_, template <class> class pi_type_>
            class MSecondaryProducer = corsika::DefaultSecondaryProducer,
            template <typename> typename pi_type_ = TStack::template pi_type>
  struct MakeView {
    using type = corsika::SecondaryView<typename TStack::stack_implementation_type,
                                        pi_type_, MSecondaryProducer>;
  };
#endif

} // namespace corsika

#include <corsika/detail/framework/stack/SecondaryView.inl>
