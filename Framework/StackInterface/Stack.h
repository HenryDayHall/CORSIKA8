#ifndef _include_Stack_h__
#define _include_Stack_h__

#include <fwk/StackIterator.h> // to help application programmres

/**
   All classes around management of particles on a stack.
 */

namespace stack {

  /**
     Interface definition of a Stack object. The Stack implements the
     std-type begin/end function to allow integration in normal for
     loops etc.
   */

  template <typename DataImpl, typename Particle>
  class Stack : public DataImpl {

  public:
    using DataImpl::GetCapacity;
    using DataImpl::GetSize;

    using DataImpl::Clear;
    using DataImpl::Copy;

    using DataImpl::DecrementSize;
    using DataImpl::IncrementSize;

  public:
    typedef Particle iterator;
    typedef const Particle const_iterator;

    /// these are functions required by std containers and std loops
    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, GetSize()); }
    iterator last() { return iterator(*this, GetSize() - 1); }

    /// these are functions required by std containers and std loops
    const_iterator cbegin() const { return const_iterator(*this, 0); }
    const_iterator cend() const { return const_iterator(*this, GetSize()); }
    const_iterator clast() const { return const_iterator(*this, GetSize() - 1); }

    /// increase stack size, create new particle at end of stack
    iterator NewParticle() {
      IncrementSize();
      return iterator(*this, GetSize() - 1);
    }
    /// delete last particle on stack by decrementing stack size
    void DeleteLast() { DecrementSize(); }
  };

} // namespace stack

#endif
