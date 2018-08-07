#ifndef _include_StackIterator_h__
#define _include_StackIterator_h__

#include <iostream>
#include <iomanip>

namespace stack {

  template<class Stack, class Particle> class StackIteratorInfo;

  /**
     The main interface to iterator over objects on a stack. 
   */
  
  template<typename Stack, typename Particle>
  class StackIterator : public Particle
  {
    friend Stack;
    friend Particle;
    friend StackIteratorInfo<Stack,Particle>;
    
  private:
    int fIndex;
    
    //#warning stacks should not be copied because of this:
    Stack* fData;
    
  public:
    StackIterator() : fData(0), fIndex(0) { }
    StackIterator(Stack& data, const int index) : fData(&data), fIndex(index) { }
    StackIterator(const StackIterator& mit) : fData(mit.fData), fIndex(mit.fIndex) { }
    
    StackIterator& operator++() { ++fIndex; return *this; }
    StackIterator operator++(int) { StackIterator tmp(*this); ++fIndex; return tmp; }
    bool operator==(const StackIterator& rhs) { return fIndex == rhs.fIndex; }
    bool operator!=(const StackIterator& rhs) { return fIndex != rhs.fIndex; }
    
    StackIterator& operator*() { return *this; }
    const StackIterator& operator*() const { return *this; }
    
  protected:
    int GetIndex() const { return fIndex; }
    Stack& GetStack() { return *fData; }
    const Stack& GetStack() const { return *fData; }
    
    inline StackIterator<Stack,Particle>& base_ref() { return static_cast<StackIterator<Stack, Particle>&>(*this); }
    inline const StackIterator<Stack,Particle>& base_ref() const { return static_cast<const StackIterator<Stack, Particle>&>(*this); }
  };

  
  /**
     Internal helper class for StackIterator     
   */
  
  template<class _Stack, class Particle>
  class StackIteratorInfo {
    
    friend Particle;  
  private:
    StackIteratorInfo() {}
    
  protected:
    inline _Stack& Stack() { return static_cast<StackIterator<_Stack, Particle>*>(this)->GetStack(); }
    inline int Index() const { return static_cast<const StackIterator<_Stack, Particle>*>(this)->GetIndex(); }
    inline const _Stack& Stack() const { return static_cast<const StackIterator<_Stack, Particle>*>(this)->GetStack(); }
  };

} // end namespace stack
  
#endif
