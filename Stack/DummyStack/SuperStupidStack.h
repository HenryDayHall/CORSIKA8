
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_superstupidstack_h_
#define _include_superstupidstack_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <string>
#include <vector>

namespace corsika::stack {

  namespace super_stupid {

    using corsika::particles::Code;
    using corsika::units::si::EnergyType;
    using corsika::units::si::operator""_GeV; // literals;

    /**
     * Example of a particle object on the stack.
     */

    template <typename _Stack>
    class ParticleRead : public StackIteratorInfo<_Stack, ParticleRead<_Stack> > {

      using StackIteratorInfo<_Stack, ParticleRead>::GetIndex;
      using StackIteratorInfo<_Stack, ParticleRead>::GetStack;

    public:
      void SetId(const Code id) { GetStack().SetId(GetIndex(), id); }
      void SetEnergy(const EnergyType& e) { GetStack().SetEnergy(GetIndex(), e); }

      Code GetId() const { return GetStack().GetId(GetIndex()); }
      const EnergyType& GetEnergy() const { return GetStack().GetEnergy(GetIndex()); }
    };

    /**
     *
     * Memory implementation of the most simple (stupid) particle stack object.
     */

    class SuperStupidStackImpl {

    public:
      void Init() {}

      void Clear() {
        fDataE.clear();
        fDataId.clear();
      }

      int GetSize() const { return fDataId.size(); }
      int GetCapacity() const { return fDataId.size(); }

      void SetId(const int i, const Code id) { fDataId[i] = id; }
      void SetEnergy(const int i, const EnergyType& e) { fDataE[i] = e; }

      const Code GetId(const int i) const { return fDataId[i]; }
      const EnergyType& GetEnergy(const int i) const { return fDataE[i]; }

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
        fDataId.push_back(Code::unknown);
      }
      void DecrementSize() {
        if (fDataE.size() > 0) {
          fDataE.pop_back();
          fDataId.pop_back();
        }
      }

    private:
      /// the actual memory to store particle data

      std::vector<Code> fDataId;
      std::vector<EnergyType> fDataE;

    }; // end class SuperStupidStackImpl

    typedef StackIterator<SuperStupidStackImpl, ParticleRead<SuperStupidStackImpl> >
        Particle;
    typedef Stack<SuperStupidStackImpl, Particle> SuperStupidStack;

  } // namespace super_stupid

} // namespace corsika::stack

#endif
