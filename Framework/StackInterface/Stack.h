
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

#include <stdexcept>

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
  class ParticleInterface; // forward decl

  /**
     The Stack class provides (and connects) the main particle data storage machinery.

     The StackData type is the user-provided bare data storage
     object. This can be of any complexity, from a simple struct
     (fortran common block), to a combination of different and
     distributed data sources.

     The user-provided ParticleInterface template type is the base
     class type of the StackIteratorInterface class (CRTP) and must
     provide all functions to read single particle data from the
     StackData, given an 'unsigned int' index.

     The Stack implements the
     std-type begin/end function to allow integration in normal for
     loops, ranges, etc.
   */

  template <typename StackData, template <typename> typename ParticleInterface>
  class Stack : public StackData {

  public:
    typedef StackData StackImpl; ///< this is the type of the user-provided data structure
    template <typename SI>
    using PIType = ParticleInterface<SI>;
    // typedef ParticleInterface<StackIteratorInterface> StackParticleInterface;  ///<
    // this is the type of the user-provided ParticleInterface typedef Stack<StackData,
    // ParticleInterface> StackType;

    /**
     * Via the StackIteratorInterface and ConstStackIteratorInterface
     * specialization, the type of the StackIterator
     * template class is declared for a particular stack data
     * object. Using CRTP, this also determines the type of
     * ParticleInterface template class simultaneously.
     */
    typedef StackIteratorInterface<StackData, ParticleInterface> StackIterator;
    typedef ConstStackIteratorInterface<StackData, ParticleInterface> ConstStackIterator;
    /**
     * this is the full type of the declared ParticleInterface: typedef typename
     */
    typedef typename StackIterator::ParticleInterfaceType ParticleType;

    friend class StackIteratorInterface<StackData, ParticleInterface>;
    friend class ConstStackIteratorInterface<StackData, ParticleInterface>;

  protected:
    using StackData::Copy;
    using StackData::Swap;

  public:
    using StackData::GetCapacity;
    using StackData::GetSize;

    using StackData::Clear;

    using StackData::DecrementSize;
    using StackData::IncrementSize;

    using StackData::Init;

  public:
    /// these are functions required by std containers and std loops
    StackIterator begin() { return StackIterator(*this, 0); }
    StackIterator end() { return StackIterator(*this, GetSize()); }
    StackIterator last() { return StackIterator(*this, GetSize() - 1); }

    ConstStackIterator begin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator end() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator last() const { return ConstStackIterator(*this, GetSize() - 1); }

    ConstStackIterator cbegin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator cend() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator clast() const { return ConstStackIterator(*this, GetSize() - 1); }

    /// increase stack size, create new particle at end of stack
    template <typename... Args>
    StackIterator AddParticle(const Args... v) {
      IncrementSize();
      return StackIterator(*this, GetSize() - 1, v...);
    }
    template <typename... Args>
    StackIterator AddSecondary(StackIterator& parent, const Args... v) {
      IncrementSize();
      return StackIterator(*this, GetSize() - 1, parent, v...);
    }
    void Swap(StackIterator a, StackIterator b) { Swap(a.GetIndex(), b.GetIndex()); }
    void Swap(ConstStackIterator a, ConstStackIterator b) {
      Swap(a.GetIndex(), b.GetIndex());
    }
    void Copy(StackIterator a, StackIterator b) { Copy(a.GetIndex(), b.GetIndex()); }
    void Copy(ConstStackIterator a, StackIterator b) { Copy(a.GetIndex(), b.GetIndex()); }
    /// delete this particle
    void Delete(StackIterator p) {
      if (GetSize() == 0) { /*error*/
        throw std::runtime_error("Stack, cannot delete entry since size is zero");
      }
      if (p.GetIndex() < GetSize() - 1) Copy(GetSize() - 1, p.GetIndex());
      DeleteLast();
      // p.SetInvalid();
    }
    void Delete(ParticleType p) { Delete(p.GetIterator()); }
    /// delete last particle on stack by decrementing stack size
    void DeleteLast() { DecrementSize(); }
    /// check if there are no further particles on stack
    bool IsEmpty() { return GetSize() == 0; }
    StackIterator GetNextParticle() { return last(); }

  protected:
    unsigned int GetIndexFromIterator(const unsigned int vI) const { return vI; }

    StackData& GetStackData() { return static_cast<StackData&>(*this); }
    const StackData& GetStackData() const { return static_cast<const StackData&>(*this); }
  };

} // namespace corsika::stack

#endif
