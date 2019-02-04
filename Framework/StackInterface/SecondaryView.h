#ifndef _include_corsika_stack_secondaryview_h_
#define _include_corsika_stack_secondaryview_h_

#include <corsika/stack/Stack.h>

#include <iostream>
#include <algorithm>
#include <vector>

namespace corsika::stack {

  /**
   * SecondaryView can only be constructed by giving a valid
   * Projectile particle, following calls to AddSecondary will
   * populate the original Stack, but will be directly accessible via
   * the SecondaryView

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
  class SecondaryView : public Stack<StackData&, ParticleInterface> {

  private:
    /**
     * Helper type for inside this class
     */
    using InnerStackType = Stack<StackData&, ParticleInterface>;

    /**
     * @name We need this "special" types with non-reference StackData for
     * the constructor of the SecondaryView class
     * @{
     */
    using InnerStackTypeV = Stack<StackData, ParticleInterface>;
    typedef StackIteratorInterface<typename std::remove_reference<StackData>::type,
                                   ParticleInterface, InnerStackTypeV>
        StackIteratorV;
    /// @}

  public:
    friend typename InnerStackType::StackIterator;
    friend typename InnerStackType::ConstStackIterator;
    
  private:
    /**
     * This is not accessible, since we don't want to allow creating a
     * new stack.
     */
    template <typename... Args>
    SecondaryView(Args... args);

  public:
    SecondaryView(StackIteratorV& vP)
        : Stack<StackData&, ParticleInterface>(vP.GetStackData())
        , fProjectileIndex(vP.GetIndex()) {}

    auto GetProjectile() {
      return typename InnerStackType::StackIterator(static_cast<InnerStackType&>(*this),
                                                    fProjectileIndex);
    }

    template <typename... Args>
    auto AddSecondary(const Args... v) {
      InnerStackType::GetStackData().IncrementSize();
      const unsigned int index = InnerStackType::GetStackData().GetSize() - 1;
      fIndices.push_back(index);
      typename InnerStackType::StackIterator proj = GetProjectile();
      return typename InnerStackType::StackIterator(static_cast<InnerStackType&>(*this),
                                                    index, proj, v...);
    }

    /**
     * overwrite Stack::GetSize to return actual number of secondaries
     */
    unsigned int GetSize() const {
      for (const auto& V : fIndices) std::cout << V << " " << std::endl;
      return fIndices.size(); }

    /**
     * @name These are functions required by std containers and std loops
     * The Stack-versions must be overwritten, since here we need the correct SecondaryView::GetSize
     * @{
     */
    auto begin() { return typename InnerStackType::StackIterator(*this, 0); }
    auto end() { return typename InnerStackType::StackIterator(*this, GetSize()); }
    auto last() { return typename InnerStackType::StackIterator(*this, GetSize() - 1); }

    auto begin() const { return typename InnerStackType::ConstStackIterator(*this, 0); }
    auto end() const { return typename InnerStackType::ConstStackIterator(*this, GetSize()); }
    auto last() const { return typename InnerStackType::ConstStackIterator(*this, GetSize() - 1); }

    auto cbegin() const { return typename InnerStackType::ConstStackIterator(*this, 0); }
    auto cend() const { return typename InnerStackType::ConstStackIterator(*this, GetSize()); }
    auto clast() const { return typename InnerStackType::ConstStackIterator(*this, GetSize() - 1); }
    /// @}

    
    /**
     * need overwrite Stack::Delete, since we want to call SecondaryView::DeleteLast
     */
    void Delete(typename InnerStackType::StackIterator p) {
      if (IsEmpty()) { /* error */
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (p.GetIndex() < GetSize() - 1) InnerStackType::GetStackData().Copy(GetSize() - 1, p.GetIndex());
      DeleteLast();
    }
    
    /**
     * need overwrite Stack::Delete, since we want to call SecondaryView::DeleteLast
     */
    void Delete(typename InnerStackType::ParticleType p) { Delete(p.GetIterator()); }

    /**
     * delete last particle on stack by decrementing stack size
     */
    void DeleteLast() {
      fIndices.pop_back();
      InnerStackType::GetStackData().DecrementSize();
    }

    /**
     * check if there are no further particles on stack
     */
    bool IsEmpty() { return GetSize() == 0; }
    
  protected:
    unsigned int GetIndexFromIterator(const unsigned int vI) const {
      std::cout << "SecondaryView::GetIndexFromIterator " << vI << " " << fIndices[vI] << std::endl;
      return fIndices[vI];
    }

  private:
    unsigned int fProjectileIndex;
    std::vector<unsigned int> fIndices;
  };

} // namespace corsika::stack

#endif
