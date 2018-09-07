#ifndef _include_stackone_h_
#define _include_stackone_h_

#include <string>
#include <vector>

#include <fwk/Stack.h>

namespace stack {

  /**
     Example of a particle object on the stack.
   */

  template <typename _Stack>
  class ParticleReadOne : public StackIteratorInfo<_Stack, ParticleReadOne<_Stack> > {
    using StackIteratorInfo<_Stack, ParticleReadOne>::GetIndex;
    using StackIteratorInfo<_Stack, ParticleReadOne>::GetStack;

  public:
    void SetId(const int id) { GetStack().SetId(GetIndex(), id); }
    void SetEnergy(const double e) { GetStack().SetEnergy(GetIndex(), e); }

    int GetId() const { return GetStack().GetId(GetIndex()); }
    double GetEnergy() const { return GetStack().GetEnergy(GetIndex()); }

    double GetPDG() const { return 0; }               // ConvertToPDG(GetId()); }
    void SetPDG(double v) { GetStack().SetId(0, 0); } // fIndex, ConvertFromPDG(v)); }
  };

  /**
     Memory implementation of the most simple particle stack object.
   */

  class StackOneImpl {
  private:
    /// the actual memory to store particle data

    std::vector<int> fId;
    std::vector<double> fData;

  public:
    void Clear() { fData.clear(); }

    int GetSize() const { return fData.size(); }
    int GetCapacity() const { return fData.size(); }

    void SetId(const int i, const int id) { fId[i] = id; }
    void SetEnergy(const int i, const double e) { fData[i] = e; }

    const int GetId(const int i) const { return fId[i]; }
    const double GetEnergy(const int i) const { return fData[i]; }

    /**
       Function to copy particle at location i2 in stack to i1
     */
    void Copy(const int i1, const int i2) {
      fData[i2] = fData[i1];
      fId[i2] = fId[i1];
    }

  protected:
    void IncrementSize() {
      fData.push_back(0.);
      fId.push_back(0.);
    }
    void DecrementSize() {
      if (fData.size() > 0) {
        fData.pop_back();
        fId.pop_back();
      }
    }
  };

  typedef StackIterator<StackOneImpl, ParticleReadOne<StackOneImpl> > ParticleOne;
  typedef Stack<StackOneImpl, ParticleOne> StackOne;

} // namespace stack

#endif
