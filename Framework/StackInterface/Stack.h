#ifndef _include_Stack_h__
#define _include_Stack_h__

#include <StackInterface/StackIterator.h> // to help application programmres

namespace stack {

  /**
     Interface definition of a Stack object.
   */
  
  template<typename DataImpl, typename Particle> 
  class Stack : public DataImpl {

  public:
    using DataImpl::GetCapacity;
    using DataImpl::GetSize;
    
    using DataImpl::Clear;
    using DataImpl::Copy;
    
    using DataImpl::IncrementSize;
    using DataImpl::DecrementSize;
    
  public:  
    typedef Particle iterator;
    typedef const Particle const_iterator;
    
    iterator begin() { return iterator(*this, 0); } 
    iterator end() { return iterator(*this, GetSize()); } 
    iterator last() { return iterator(*this, GetSize()-1); } 
    
    const_iterator cbegin() const { return const_iterator(*this, 0); } 
    const_iterator cend() const { return const_iterator(*this, GetSize()); } 
    const_iterator clast() const { return const_iterator(*this, GetSize()-1); } 
    
    iterator NewParticle() { IncrementSize(); return iterator(*this, GetSize()-1); }
    void DeleteLast() { DecrementSize(); }
  };

} // end namespace
  
#endif
  
