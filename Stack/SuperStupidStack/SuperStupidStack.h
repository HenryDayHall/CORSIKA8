
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

#include <vector>

namespace corsika::stack {

  namespace super_stupid {

    using corsika::particles::Code;
    using corsika::units::si::EnergyType;
    using corsika::units::si::operator""_GeV; // literals;

    /**
     * Example of a particle object on the stack.
     */

    template <typename StackIteratorInterface>
    class ParticleInterface : public ParticleBase<StackIteratorInterface> {

      using ParticleBase<StackIteratorInterface>::GetStackData;
      using ParticleBase<StackIteratorInterface>::GetIndex;

    public:
      void SetPID(const Code id) { GetStackData().SetPID(GetIndex(), id); }
      void SetEnergy(const EnergyType& e) { GetStackData().SetEnergy(GetIndex(), e); }

      Code GetPID() const { return GetStackData().GetPID(GetIndex()); }
      EnergyType GetEnergy() const { return GetStackData().GetEnergy(GetIndex()); }
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
        fDataPID.clear();
      }

      int GetSize() const { return fDataPID.size(); }
      int GetCapacity() const { return fDataPID.size(); }

      void SetPID(const int i, const Code id) { fDataPID[i] = id; }
      void SetEnergy(const int i, const EnergyType e) { fDataE[i] = e; }

      Code GetPID(const int i) const { return fDataPID[i]; }
      EnergyType GetEnergy(const int i) const { return fDataE[i]; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Copy(const int i1, const int i2) {
        fDataE[i2] = fDataE[i1];
        fDataPID[i2] = fDataPID[i1];
      }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Swap(const int i1, const int i2) {
        EnergyType tE = fDataE[i2];
        Code tC = fDataPID[i2];
        fDataE[i2] = fDataE[i1];
        fDataPID[i2] = fDataPID[i1];
        fDataE[i1] = tE;
        fDataPID[i1] = tC;
      }

    protected:
      void IncrementSize() {
        fDataE.push_back(0_GeV);
        fDataPID.push_back(Code::Unknown);
      }
      void DecrementSize() {
        if (fDataE.size() > 0) {
          fDataE.pop_back();
          fDataPID.pop_back();
        }
      }

    private:
      /// the actual memory to store particle data

      std::vector<Code> fDataPID;
      std::vector<EnergyType> fDataE;

    }; // end class SuperStupidStackImpl

    typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;

  } // namespace super_stupid

} // namespace corsika::stack

#endif
