/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Stack_h__
#define _include_Stack_h__

#include <corsika/stack/StackIteratorInterface.h>
// must be after StackIteratorInterface
#include <corsika/stack/SecondaryView.h>
#include <corsika/utl/MetaProgramming.h>

#include <stdexcept>
#include <type_traits>

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

  template <typename StackDataType, template <typename> typename ParticleInterface>
  class Stack {
    using StackDataValueType = std::remove_reference_t<StackDataType>;

    StackDataType fData; ///< this in general holds all the data and can be quite big

  private:
    Stack(Stack&) = delete; ///< since Stack can be very big, we don't want to copy it
    Stack& operator=(Stack&) =
        delete; ///< since Stack can be very big, we don't want to copy it

  public:
    // Stack() { Init(); }

    /**
     * if StackDataType is a reference member we *HAVE* to initialize
     * it in the constructor, this is typically needed for SecondaryView
     */
    template <typename _ = StackDataType, typename = utl::enable_if<std::is_reference<_>>>
    Stack(StackDataType vD)
        : fData(vD) {}

    /**
     * This constructor takes any argument and passes it on to the
     * StackDataType user class. If the user did not provide a suited
     * constructor this will fail with an error message.
     *
     * Furthermore, this is disabled with enable_if for SecondaryView
     * stacks, where the inner data container is always a reference
     * and cannot be initialized here.
     */
    template <typename... Args, typename _ = StackDataType,
              typename = utl::disable_if<std::is_reference<_>>>
    Stack(Args... args)
        : fData(args...) {}

  public:
    typedef StackDataType
        StackImpl; ///< this is the type of the user-provided data structure

    template <typename SI>
    using PIType = ParticleInterface<SI>;

    /**
     * Via the StackIteratorInterface and ConstStackIteratorInterface
     * specialization, the type of the StackIterator
     * template class is declared for a particular stack data
     * object. Using CRTP, this also determines the type of
     * ParticleInterface template class simultaneously.
     */
    using StackIterator =
        StackIteratorInterface<StackDataValueType, ParticleInterface, Stack>;
    using ConstStackIterator =
        ConstStackIteratorInterface<StackDataValueType, ParticleInterface, Stack>;

    /**
     * this is the full type of the user-declared ParticleInterface
     */
    using ParticleInterfaceType = typename StackIterator::ParticleInterfaceType;
    /**
     * In all programming context, the object to access, copy, and
     * transport particle data is via the StackIterator
     */
    using ParticleType = StackIterator;

    // friends are needed since they need access to protected members
    friend class StackIteratorInterface<StackDataValueType, ParticleInterface, Stack>;
    friend class ConstStackIteratorInterface<StackDataValueType, ParticleInterface,
                                             Stack>;

  public:
    /**
     * @name Most generic proxy methods for StackDataType fData
     * @{
     */
    unsigned int GetCapacity() const { return fData.GetCapacity(); }
    unsigned int GetSize() const { return fData.GetSize(); }
    template <typename... Args>
    auto Init(Args... args) {
      return fData.Init(args...);
    }
    template <typename... Args>
    auto Clear(Args... args) {
      return fData.Clear(args...);
    }
    ///@}

  public:
    /**
     * @name These are functions required by std containers and std loops
     * @{
     */
    StackIterator begin() { return StackIterator(*this, 0); }
    StackIterator end() { return StackIterator(*this, GetSize()); }
    StackIterator last() { return StackIterator(*this, GetSize() - 1); }

    ConstStackIterator begin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator end() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator last() const { return ConstStackIterator(*this, GetSize() - 1); }

    ConstStackIterator cbegin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator cend() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator clast() const { return ConstStackIterator(*this, GetSize() - 1); }
    /// @}

    /**
     * increase stack size, create new particle at end of stack
     */
    template <typename... Args>
    StackIterator AddParticle(const Args... v) {
      fData.IncrementSize();
      return StackIterator(*this, GetSize() - 1, v...);
    }

    /**
     * increase stack size, create new particle at end of stack, related to parent
     * particle/projectile
     */
    template <typename... Args>
    StackIterator AddSecondary(StackIterator& parent, const Args... v) {
      fData.IncrementSize();
      return StackIterator(*this, GetSize() - 1, parent, v...);
    }

    void Swap(StackIterator a, StackIterator b) {
      fData.Swap(a.GetIndex(), b.GetIndex());
    }
    void Swap(ConstStackIterator a, ConstStackIterator b) {
      fData.Swap(a.GetIndex(), b.GetIndex());
    }
    void Copy(StackIterator a, StackIterator b) {
      fData.Copy(a.GetIndex(), b.GetIndex());
    }
    void Copy(ConstStackIterator a, StackIterator b) {
      fData.Copy(a.GetIndex(), b.GetIndex());
    }

    /**
     * delete this particle
     */
    void Delete(StackIterator p) {
      if (GetSize() == 0) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (p.GetIndex() < GetSize() - 1) fData.Copy(GetSize() - 1, p.GetIndex());
      DeleteLast();
    }
    /**
     * delete this particle
     */
    void Delete(ParticleInterfaceType p) { Delete(p.GetIterator()); }

    /**
     * delete last particle on stack by decrementing stack size
     */
    void DeleteLast() { fData.DecrementSize(); }

    /**
     * check if there are no further particles on stack
     */
    bool IsEmpty() { return GetSize() == 0; }

    /**
     * return next particle from stack
     */
    StackIterator GetNextParticle() { return last(); }

  protected:
    /**
     * Function to perform eventual transformation from
     * StackIterator::GetIndex() to index in data stored in
     * StackDataType fData. By default (and in almost all cases) this
     * should just be identiy. See class SecondaryView for an alternative implementation.
     */
    unsigned int GetIndexFromIterator(const unsigned int vI) const { return vI; }

    /**
     * @name Return reference to StackDataType object fData for data access
     * @{
     */
    StackDataValueType& GetStackData() { return fData; }
    const StackDataValueType& GetStackData() const { return fData; }
    ///@}
  };

} // namespace corsika::stack

#endif
