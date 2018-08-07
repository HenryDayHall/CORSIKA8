#ifndef _include_stackone_h_
#define _include_stackone_h_

#include <vector>
#include <string>

#include <StackInterface/Stack.h>


namespace stack {
  
  
  /**
     Example of a particle object on the stack.
   */
  
  template<typename _Stack>
  class ParticleReadOne : public StackIteratorInfo<_Stack, ParticleReadOne<_Stack> >
    {
      using StackIteratorInfo<_Stack, ParticleReadOne>::Index;
      using StackIteratorInfo<_Stack, ParticleReadOne>::Stack;
      
    public:
      void SetId(const int id) { Stack().SetId(Index(), id); }
      void SetEnergy(const double e) { Stack().SetEnergy(Index(), e); }
      
      int GetId() const { Stack().GetId(Index()); }
      double GetEnergy() const { Stack().GetEnergy(Index()); }
      
      double GetPDG() const { return 0; } // ConvertToPDG(GetId()); }  
      void SetPDG(double v) { Stack().SetId(0, 0); } //fIndex, ConvertFromPDG(v)); }
    };
  
  
  /**
     Definition of one most simple particle stack object.
   */
  
  class StackOneImpl
  {    
  private:
    std::vector<int> fId;
    std::vector<double> fData;
    
  public:
    void Clear() { fData.clear(); }
    
    int GetSize() const { return fData.size();  }
    int GetCapacity() const { return fData.size(); }
    
    
    void SetId(const int i, const int id) { fId[i] = id; }
    void SetEnergy(const int i, const double e) { fData[i] = e; }
    
    const int GetId(const int i) const { return fId[i]; }
    const double GetEnergy(const int i) const { return fData[i]; }
    
    void Copy(const int i1, const int i2) {
      fData[i2] = fData[i1];
      fId[i2] = fId[i1];
    }
    
  protected:
    void IncrementSize() { fData.push_back(0.); fId.push_back(0.); }
    void DecrementSize() { if (fData.size()>0) { fData.pop_back(); fId.pop_back(); } }
  };
  
  typedef StackIterator<StackOneImpl, ParticleReadOne<StackOneImpl> > ParticleOne;
  typedef Stack<StackOneImpl, ParticleOne> StackOne;
  
} // end namespace
  
#endif
