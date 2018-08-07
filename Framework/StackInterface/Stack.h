#ifndef _include_Stack_h__
#define _include_Stack_h__

#include <StackIterator.h> // to help application programmres

namespace stack {

  /**
     Interface definition of a Stack object.
   */
  
  template<typename DataImpl, typename Particle> 
  class Stack : public DataImpl {

  public:
    using DataImpl::Capacity;
    using DataImpl::Size;
    
    using DataImpl::Clear;
    using DataImpl::Copy;
    
    using DataImpl::IncrementSize;
    using DataImpl::DecrementSize;
    
  public:  
    typedef Particle iterator;
    typedef const Particle const_iterator;
    
    iterator Begin() { return iterator(*this, 0); } 
    iterator End() { return iterator(*this, Size()); } 
    iterator Last() { return iterator(*this, Size()-1); } 
    
    const_iterator CBegin() const { return const_iterator(*this, 0); } 
    const_iterator CEnd() const { return const_iterator(*this, Size()); } 
    const_iterator CLast() const { return const_iterator(*this, Size()-1); } 
    
    iterator NewParticle() { IncrementSize(); return iterator(*this, Size()-1); }
    void DeleteLast() { DecrementSize(); }
  };

} // end namespace
  
#endif
  
