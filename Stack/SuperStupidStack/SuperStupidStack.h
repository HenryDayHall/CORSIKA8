#ifndef _include_superstupidstack_h_
#define _include_superstupidstack_h_

#include <string>
#include <vector>

#include <fwk/ParticleProperties.h>
#include <fwk/PhysicalUnits.h>
#include <fwk/Stack.h>

using namespace fwk::literals;

namespace stack {

  namespace super_stupid {

    /**
     * Example of a particle object on the stack.
     */

    template <typename _Stack>
    class ParticleRead : public StackIteratorInfo<_Stack, ParticleRead<_Stack> > {

      using StackIteratorInfo<_Stack, ParticleRead>::GetIndex;
      using StackIteratorInfo<_Stack, ParticleRead>::GetStack;

    public:
      void SetId(const fwk::particle::Code id) { GetStack().SetId(GetIndex(), id); }
      void SetEnergy(const fwk::Energy& e) { GetStack().SetEnergy(GetIndex(), e); }

      fwk::particle::Code GetId() const { return GetStack().GetId(GetIndex()); }
      const fwk::Energy& GetEnergy() const { return GetStack().GetEnergy(GetIndex()); }
    };

    /**
     *
     * Memory implementation of the most simple (stupid) particle stack object.
     */

    class SuperStupidStackImpl {

    public:
      void Clear() {
        fDataE.clear();
        fDataId.clear();
      }

      int GetSize() const { return fDataId.size(); }
      int GetCapacity() const { return fDataId.size(); }

      void SetId(const int i, const fwk::particle::Code id) { fDataId[i] = id; }
      void SetEnergy(const int i, const fwk::Energy& e) { fDataE[i] = e; }

      const fwk::particle::Code GetId(const int i) const { return fDataId[i]; }
      const fwk::Energy& GetEnergy(const int i) const { return fDataE[i]; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Copy(const int i1, const int i2) {
        fDataE[i2] = fDataE[i1];
        fDataId[i2] = fDataId[i1];
      }

    protected:
      void IncrementSize() {
        fDataE.push_back(0_GeV);
        fDataId.push_back(fwk::particle::Code::unknown);
      }
      void DecrementSize() {
        if (fDataE.size() > 0) {
          fDataE.pop_back();
          fDataId.pop_back();
        }
      }

    private:
      /// the actual memory to store particle data

      std::vector<fwk::particle::Code> fDataId;
      std::vector<fwk::Energy> fDataE;

    }; // end class SuperStupidStackImpl

    typedef StackIterator<SuperStupidStackImpl, ParticleRead<SuperStupidStackImpl> >
        Particle;
    typedef Stack<SuperStupidStackImpl, Particle> SuperStupidStack;

  } // namespace super_stupid

} // namespace stack

#endif
