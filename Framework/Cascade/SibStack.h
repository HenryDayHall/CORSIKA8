#ifndef _include_sibstack_h_
#define _include_sibstack_h_

#include <vector>
#include <string>

#include <corsika/stack/Stack.h>
#include <corsika/cascade/sibyll2.3c.h>
#include <corsika/process/sibyll/ParticleConversion.h>

using namespace std;
using namespace corsika::stack;

class SibStackData {
  
 public:
  void Init();
  
  void Clear() { s_plist_.np = 0; }
  
  int GetSize() const { return s_plist_.np;  }
  int GetCapacity() const { return 8000; }

  
  void SetId(const int i, const int v) { s_plist_.llist[i]=v; }
  void SetEnergy(const int i, const double v) { s_plist_.p[3][i]=v;  }

  int GetId(const int i) const { return s_plist_.llist[i]; }
  double GetEnergy(const int i) const { return s_plist_.p[3][i]; }
  
  void Copy(const int i1, const int i2) {
    s_plist_.llist[i1] = s_plist_.llist[i2];
    s_plist_.p[3][i1] = s_plist_.p[3][i2];
  }
  
 protected:
  void IncrementSize() { s_plist_.np++; }
  void DecrementSize() { if ( s_plist_.np>0) { s_plist_.np--; } }
};


template<typename StackIteratorInterface>
class ParticleInterface : public ParticleBase<StackIteratorInterface> {
  using ParticleBase<StackIteratorInterface>::GetStackData;
  using ParticleBase<StackIteratorInterface>::GetIndex;
 public:
  void SetEnergy(const double v) { GetStackData().SetEnergy(GetIndex(), v); }
  double GetEnergy() const { return GetStackData().GetEnergy(GetIndex()); }
  void SetPID(const int v) { GetStackData().SetId(GetIndex(), v); }
  corsika::process::sibyll::SibyllCode GetPID() const { return static_cast<corsika::process::sibyll::SibyllCode> (GetStackData().GetId(GetIndex())); }
  
};


typedef Stack<SibStackData, ParticleInterface> SibStack;

#endif
