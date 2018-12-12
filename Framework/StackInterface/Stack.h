
/**
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

#include <corsika/stack/StackIterator.h> // include here, to help application programmres

/**
   All classes around management of particles on a stack.
 */

namespace corsika::stack {

  template <typename>
  class PI; // forward decl

  /**
     Interface definition of a Stack object. The Stack implements the
     std-type begin/end function to allow integration in normal for
     loops etc.
   */

  template <typename StackData, template <typename> typename PI>
  class Stack : public StackData {

  public:
    typedef Stack<StackData, PI> StackType;
    typedef StackIteratorInterface<StackData, PI> StackIterator;
    typedef const StackIterator ConstStackIterator;
    typedef typename StackIterator::ParticleInterfaceType ParticleType;
    friend class StackIteratorInterface<StackData, PI>;

  public:
    using StackData::GetCapacity;
    using StackData::GetSize;

    using StackData::Clear;
    using StackData::Copy;

    using StackData::DecrementSize;
    using StackData::IncrementSize;

    using StackData::Init;

  public:
    /// these are functions required by std containers and std loops
    StackIterator begin() { return StackIterator(*this, 0); }
    StackIterator end() { return StackIterator(*this, GetSize()); }
    StackIterator last() { return StackIterator(*this, GetSize() - 1); }

    /// these are functions required by std containers and std loops
    ConstStackIterator cbegin() const { return ConstStackIterator(*this, 0); }
    ConstStackIterator cend() const { return ConstStackIterator(*this, GetSize()); }
    ConstStackIterator clast() const { return ConstStackIterator(*this, GetSize() - 1); }

    /// increase stack size, create new particle at end of stack
    StackIterator NewParticle() {
      IncrementSize();
      return StackIterator(*this, GetSize() - 1);
    }
    void Copy(StackIterator& a, StackIterator& b) { Copy(a.GetIndex(), b.GetIndex()); }
    /// delete this particle
    void Delete(StackIterator& p) {
      if (GetSize() == 0) { /*error*/
      }
      if (p.GetIndex() < GetSize() - 1) Copy(GetSize() - 1, p.GetIndex());
      DeleteLast();
      // p.SetInvalid();
    }
    void Delete(ParticleType& p) { Delete(p.GetIterator()); }
    /// delete last particle on stack by decrementing stack size
    void DeleteLast() { DecrementSize(); }
    /// check if there are no further particles on stack
    bool IsEmpty() { return GetSize() == 0; }
    StackIterator GetNextParticle() { return last(); }

  protected:
    StackData& GetStackData() { return static_cast<StackData&>(*this); }
    const StackData& GetStackData() const { return static_cast<const StackData&>(*this); }
  };

} // namespace corsika::stack

#endif
