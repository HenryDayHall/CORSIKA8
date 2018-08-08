#ifndef _include_StackIterator_h__
#define _include_StackIterator_h__

#include <iostream>
#include <iomanip>

namespace stack {

  // forward decl.
  template<class Stack, class Particle> class StackIteratorInfo;

  /**
     \class StackIterator
     
     The StackIterator is the main interface to iterator over
     particles on a stack. At the same time StackIterator is a
     Particle object by itself, thus there is no difference between
     type and ref_type for convenience of the physicist.

     This allows to write code like
     \verbatim
     for (auto& p : theStack) { p.SetEnergy(newEnergy); }  
     \endverbatim     
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

    // this is probably not needed rigth now:
    //inline StackIterator<Stack,Particle>& BaseRef() { return static_cast<StackIterator<Stack, Particle>&>(*this); }
    //inline const StackIterator<Stack,Particle>& BaseRef() const { return static_cast<const StackIterator<Stack, Particle>&>(*this); }
  };

  

  /**
     \class StackIteratorInfo
     
     Internal helper class for StackIterator. Document better...
   */
  
  template<typename _Stack, typename Particle>
  class StackIteratorInfo {
    
    friend Particle;  
  private:
    StackIteratorInfo() {}
    
  protected:
    inline _Stack& GetStack() { return static_cast<StackIterator<_Stack, Particle>*>(this)->GetStack(); }
    inline int GetIndex() const { return static_cast<const StackIterator<_Stack, Particle>*>(this)->GetIndex(); }
    inline const _Stack& GetStack() const { return static_cast<const StackIterator<_Stack, Particle>*>(this)->GetStack(); }
  };

} // end namespace stack
  
#endif
