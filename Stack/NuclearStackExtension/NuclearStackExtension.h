
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_stack_nuclearstackextension_h_
#define _include_stack_nuclearstackextension_h_

#include <corsika/stack/super_stupid/SuperStupidStack.h>

#include <vector>

namespace corsika::stack {

  namespace nuclear_extension {

    using corsika::stack::super_stupid::MomentumVector;
    
    template <typename StackIteratorInterface>
    class ParticleInterface
        : public corsika::stack::super_stupid::ParticleInterface<StackIteratorInterface> {

    protected:
      //      using corsika::stack::ParticleBase<StackIteratorInterface>::GetStack;
      //using corsika::stack::super_stupid::ParticleInterface<StackIteratorInterface>::GetStackData;
      using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
      using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

    public:
      void SetParticleData(const corsika::particles::Code vDataPID,
                           const corsika::units::si::HEPEnergyType vDataE,
                           const corsika::stack::super_stupid::MomentumVector& vMomentum,
                           const corsika::geometry::Point& vPosition,
                           const corsika::units::si::TimeType vTime,
			   const int vA = 0,
			   const int vZ = 0) {
	if (vDataPID == corsika::particles::Code::Nucleus) {
	  if (vA==0 || vZ==0) {
	    std::ostringstream err;
	    err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
	    throw std::runtime_error(err.str());
	  }
	  SetNuclearRef(corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData().GetNucleusNextRef()); // store this nucleus data ref
	  SetNuclearA(vA);
	  SetNuclearZ(vZ);
	} else {
	  SetNuclearRef(-1); // this is not a nucleus
	}
        corsika::stack::super_stupid::ParticleInterface<StackIteratorInterface>::SetParticleData(vDataPID,
												 vDataE,
												 vMomentum,
												 vPosition,
												 vTime);
      }

      void SetParticleData(ParticleInterface<StackIteratorInterface>& parent,
                           const corsika::particles::Code vDataPID,
                           const corsika::units::si::HEPEnergyType vDataE,
                           const corsika::stack::super_stupid::MomentumVector& vMomentum,
                           const corsika::geometry::Point& vPosition,
                           const corsika::units::si::TimeType vTime,
			   const int vA = 0,
			   const int vZ = 0) {
	SetParticleData(vDataPID,
                        vDataE,
                        vMomentum,
                        vPosition,
			vTime,
			vA,
			vZ);
      }
      
      /**
       * @name individual setters
       * @{
       */
      void SetNuclearA(const int vA) { GetStackData().SetNuclearA(GetIndex(), vA); }
      void SetNuclearZ(const int vZ) { GetStackData().SetNuclearZ(GetIndex(), vZ); }      
      /// @}

      /**
       * @name individual getters
       * @{
       */
      int GetNuclearA() const { return GetStackData().GetNuclearA(GetIndex()); }
      int GetNuclearZ() const { return GetStackData().GetNuclearZ(GetIndex()); }
      /// @}

    protected:
      void SetNuclearRef(const int vR) { GetStackData().SetNuclearRef(GetIndex(), vR); }
    };

    
    /**
     * Memory implementation of the most simple (stupid) particle stack object.
     */

    class NuclearStackExtensionImpl
        : public corsika::stack::super_stupid::SuperStupidStackImpl {

    public:
      void Init() { corsika::stack::super_stupid::SuperStupidStackImpl::Init(); }

      void Clear() {
        corsika::stack::super_stupid::SuperStupidStackImpl::Clear();
        fNuclearRef.clear();
        fNuclearA.clear();
        fNuclearZ.clear();
      }

      int GetSize() const { return fNuclearRef.size(); }
      int GetCapacity() const { return fNuclearRef.size(); }

      void SetNuclearA(const int i, const int vA) { fNuclearA[GetNucleusRef(i)] = vA; }
      void SetNuclearZ(const int i, const int vZ) { fNuclearZ[GetNucleusRef(i)] = vZ; }
      void SetNuclearRef(const int i, const int v) { fNuclearRef[i] = v; }
      
      int GetNuclearA(const int i) const { return fNuclearA[GetNucleusRef(i)]; }
      int GetNuclearZ(const int i) const { return fNuclearZ[GetNucleusRef(i)]; }
      // this function will create new storage for Nuclear Properties, and return the reference to it
      int GetNucleusNextRef() { fNuclearA.push_back(0); fNuclearZ.push_back(0); return fNuclearA.size()-1; }

      int GetNucleusRef(const int i) const {
	if (fNuclearRef[i]>=0)
	  return fNuclearRef[i];
	std::ostringstream err;
	err << "NuclearStackExtension: no nucleus at ref=" << i;
	throw std::runtime_error(err.str());
      }
      
      /**
       *   Function to copy particle at location i1 in stack to i2
       */
      void Copy(const int i1, const int i2) {
        corsika::stack::super_stupid::SuperStupidStackImpl::Copy(i1, i2);
	const int ref1 = fNuclearRef[i1];
	const int ref2 = fNuclearRef[i2];
	if (ref2<0) {
	  if (ref1>=0) {
	    // i1 is nucleus, i2 is not
	    fNuclearRef[i2] = GetNucleusNextRef();
	    fNuclearA[fNuclearRef[i2]] = fNuclearA[ref1];
	    fNuclearZ[fNuclearRef[i2]] = fNuclearZ[ref1];
	  } else {
	    // neither i1 nor i2 are nuclei
	  }
	} else {
	  if (ref1>=0) {
	    // both are nuclei, i2 is overwritten with nucleus i1
	    fNuclearA[ref2] = fNuclearA[ref1];
	    fNuclearZ[ref2] = fNuclearZ[ref1];
	  } else {
	    // i2 is overwritten with non-nucleus i1
	    fNuclearA.erase(fNuclearA.begin() + ref2);
	    fNuclearZ.erase(fNuclearZ.begin() + ref2);
	    const int n = fNuclearRef.size();
	    for (int i=0; i<n; ++i) {
	      if (fNuclearRef[i]>=ref2) {
		fNuclearRef[i] -= 1;
	      }
	    }
	  }
	}
      }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Swap(const int i1, const int i2) {
        corsika::stack::super_stupid::SuperStupidStackImpl::Swap(i1, i2);
	const int ref1 = fNuclearRef[i1];
	const int ref2 = fNuclearRef[i2];
	if (ref2<0) {
	  if (ref1>=0) {
	    // i1 is nucleus, i2 is not
	    std::swap(fNuclearRef[i2], fNuclearRef[i1]);
	  } else {
	    // neither i1 nor i2 are nuclei
	  }
	} else {
	  if (ref1>=0) {
	    // both are nuclei, i2 is overwritten with nucleus i1
	    std::swap(fNuclearRef[i2], fNuclearRef[i1]);
	  } else {
	    // i2 is overwritten with non-nucleus i1
	    std::swap(fNuclearRef[i2], fNuclearRef[i1]);
	  }
	}
      }

    protected:
      void IncrementSize() {
        corsika::stack::super_stupid::SuperStupidStackImpl::IncrementSize();
        fNuclearRef.push_back(-1);
      }

      void DecrementSize() {
        corsika::stack::super_stupid::SuperStupidStackImpl::DecrementSize();
        if (fNuclearRef.size() > 0) {
	  const int ref = fNuclearRef.back();
	  fNuclearRef.pop_back();
	  if (ref>=0) {
	    fNuclearA.erase(fNuclearA.begin() + ref);
	    fNuclearZ.erase(fNuclearZ.begin() + ref);
	    const int n = fNuclearRef.size();
	    for (int i=0; i<n; ++i) {
	      if (fNuclearRef[i] >= ref) {
		fNuclearRef[i] -= 1;
	      }
	    }	    
	  }
	}
      }

    private:
      /// the actual memory to store particle data

      std::vector<int> fNuclearRef;
      std::vector<int> fNuclearA;
      std::vector<int> fNuclearZ;

    }; // end class SuperStupidStackImpl

    typedef Stack<NuclearStackExtensionImpl, ParticleInterface> NuclearStackExtension;

  } // namespace nuclear_extension
} // namespace corsika::stack

#endif
