#ifndef _include_corsika_stack_secondaryview_h_
#define _include_corsika_stack_secondaryview_h_

#include <corsika/stack/Stack.h>

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
   */

  /* INTERNAL NOTE FOR DEVELOPERS The secondary particle indices are
     stored in a std::vector fIndices, the index of the primary projectle
     particle is explicitly stored in fProjectileIndex. StackIterator
     indices are refering to those numbers, where
     StackIterator::GetIndex()==0 refers to the fProjectileIndex and
     StackIterator::GetIndex()>0 to fIndices[i+1], see function GetIndexFromIterator.
   */

  template <typename StackDataType, template <typename> typename ParticleInterface>
  class SecondaryView : public Stack<StackDataType&, ParticleInterface> {

    using ViewType = SecondaryView<StackDataType, ParticleInterface>;

  private:
    /**
     * Helper type for inside this class
     */
    using InnerStackType = Stack<StackDataType&, ParticleInterface>;

    /**
     * @name We need this "special" types with non-reference StackData for
     * the constructor of the SecondaryView class
     * @{
     */
    using InnerStackTypeV = Stack<StackDataType, ParticleInterface>;
    typedef StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                   ParticleInterface, InnerStackTypeV>
        StackIteratorV;
    /// @}

  public:
    using StackIterator =
        StackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                               ParticleInterface, ViewType>;
    using ConstStackIterator =
        ConstStackIteratorInterface<typename std::remove_reference<StackDataType>::type,
                                    ParticleInterface, ViewType>;

    /**
     * this is the full type of the declared ParticleInterface: typedef typename
     */
    using ParticleType = typename StackIterator::ParticleInterfaceType;

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
    SecondaryView(Args... args);

  public:
    SecondaryView(StackIteratorV& vP)
        : Stack<StackDataType&, ParticleInterface>(vP.GetStackData())
        , fProjectileIndex(vP.GetIndex()) {}

    auto GetProjectile() {
      // NOTE: 0 is special marker here for PROJECTILE, see GetIndexFromIterator
      return StackIterator(*this, 0);
    }

    template <typename... Args>
    auto AddSecondary(const Args... v) {
      InnerStackType::GetStackData().IncrementSize();
      const unsigned int idSec = GetSize();
      const unsigned int index = InnerStackType::GetStackData().GetSize() - 1;
      fIndices.push_back(index);
      StackIterator proj = GetProjectile();
      // NOTE: "+1" is since "0" is special marker here for PROJECTILE, see
      // GetIndexFromIterator
      return StackIterator(*this, idSec + 1, proj, v...);
    }

    /**
     * overwrite Stack::GetSize to return actual number of secondaries
     */
    unsigned int GetSize() const { return fIndices.size(); }

    /**
     * @name These are functions required by std containers and std loops
     * The Stack-versions must be overwritten, since here we need the correct
     * SecondaryView::GetSize
     * @{
     */
    // NOTE: the "+1" is since "0" is special marker here for PROJECTILE, see
    // GetIndexFromIterator
    auto begin() { return StackIterator(*this, 0 + 1); }
    auto end() { return StackIterator(*this, GetSize() + 1); }
    auto last() { return StackIterator(*this, GetSize() - 1 + 1); }

    auto begin() const { return ConstStackIterator(*this, 0 + 1); }
    auto end() const { return ConstStackIterator(*this, GetSize() + 1); }
    auto last() const { return ConstStackIterator(*this, GetSize() - 1 + 1); }

    auto cbegin() const { return ConstStackIterator(*this, 0 + 1); }
    auto cend() const { return ConstStackIterator(*this, GetSize() + 1); }
    auto clast() const { return ConstStackIterator(*this, GetSize() - 1 + 1); }
    /// @}

    /**
     * need overwrite Stack::Delete, since we want to call SecondaryView::DeleteLast
     */
    void Delete(StackIterator p) {
      if (IsEmpty()) { /* error */
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (p.GetIndex() < GetSize() - 1)
        InnerStackType::GetStackData().Copy(GetSize() - 1, p.GetIndex());
      DeleteLast();
    }

    /**
     * need overwrite Stack::Delete, since we want to call SecondaryView::DeleteLast
     */
    void Delete(ParticleType p) { Delete(p.GetIterator()); }

    /**
     * delete last particle on stack by decrementing stack size
     */
    void DeleteLast() {
      fIndices.pop_back();
      InnerStackType::GetStackData().DecrementSize();
    }

    /**
     * return next particle from stack, need to overwrtie Stack::GetNextParticle to get
     * right reference
     */
    StackIterator GetNextParticle() { return last(); }

    /**
     * check if there are no further particles on stack
     */
    bool IsEmpty() { return GetSize() == 0; }

  protected:
    /**
     * We only want to 'see' secondaries indexed in fIndices. In this
     * function the conversion form iterator-index to stack-index is
     * performed.
     */
    unsigned int GetIndexFromIterator(const unsigned int vI) const {
      if (vI == 0) return fProjectileIndex;
      return fIndices[vI - 1];
    }

  private:
    unsigned int fProjectileIndex;
    std::vector<unsigned int> fIndices;
  };

} // namespace corsika::stack

#endif
