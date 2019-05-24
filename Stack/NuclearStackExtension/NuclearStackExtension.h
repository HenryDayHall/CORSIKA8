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

#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>

#include <algorithm>
#include <tuple>
#include <vector>

namespace corsika::stack {

  /**
   * @namespace nuclear_extension
   *
   * Add A and Z data to existing stack of particle properties.
   *
   * Only for Code::Nucleus particles A and Z are stored, not for all
   * normal elementary particles.
   *
   * Thus in your code, make sure to always check <code>
   * particle.GetPID()==Code::Nucleus </code> before attempting to
   * read any nuclear information.
   *
   *
   */

  typedef corsika::geometry::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

  namespace nuclear_extension {

    /**
     * @class NuclearParticleInterface
     *
     * Define ParticleInterface for NuclearStackExtension Stack derived from
     * ParticleInterface of Inner stack class
     */
    template <template <typename> typename InnerParticleInterface,
              typename StackIteratorInterface>
    class NuclearParticleInterface
        : public InnerParticleInterface<StackIteratorInterface> {

    protected:
      using InnerParticleInterface<StackIteratorInterface>::GetStackData;
      using InnerParticleInterface<StackIteratorInterface>::GetIndex;

    public:
      void SetParticleData(
          const std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                           corsika::stack::MomentumVector, corsika::geometry::Point,
                           corsika::units::si::TimeType>& v) {
        if (std::get<0>(v) == corsika::particles::Code::Nucleus) {
          std::ostringstream err;
          err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
          throw std::runtime_error(err.str());
        }
        InnerParticleInterface<StackIteratorInterface>::SetParticleData(v);
        SetNucleusRef(-1); // this is not a nucleus
      }

      void SetParticleData(
          const std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                           corsika::stack::MomentumVector, corsika::geometry::Point,
                           corsika::units::si::TimeType, unsigned short, unsigned short>&
              v) {
        const unsigned short A = std::get<5>(v);
        const unsigned short Z = std::get<6>(v);
        if (std::get<0>(v) != corsika::particles::Code::Nucleus || A == 0 || Z == 0) {
          std::ostringstream err;
          err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
          throw std::runtime_error(err.str());
        }
        SetNucleusRef(GetStackData().GetNucleusNextRef()); // store this nucleus data ref
        SetNuclearA(A);
        SetNuclearZ(Z);
        InnerParticleInterface<StackIteratorInterface>::SetParticleData(
            std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                       corsika::stack::MomentumVector, corsika::geometry::Point,
                       corsika::units::si::TimeType>{std::get<0>(v), std::get<1>(v),
                                                     std::get<2>(v), std::get<3>(v),
                                                     std::get<4>(v)});
      }

      void SetParticleData(
          InnerParticleInterface<StackIteratorInterface>& p,
          const std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                           corsika::stack::MomentumVector, corsika::geometry::Point,
                           corsika::units::si::TimeType>& v) {
        if (std::get<0>(v) == corsika::particles::Code::Nucleus) {
          std::ostringstream err;
          err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
          throw std::runtime_error(err.str());
        }
        InnerParticleInterface<StackIteratorInterface>::SetParticleData(
            p, std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                          corsika::stack::MomentumVector, corsika::geometry::Point,
                          corsika::units::si::TimeType>{std::get<0>(v), std::get<1>(v),
                                                        std::get<2>(v), std::get<3>(v),
                                                        std::get<4>(v)});
        SetNucleusRef(-1); // this is not a nucleus
      }

      void SetParticleData(
          InnerParticleInterface<StackIteratorInterface>& p,
          const std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                           corsika::stack::MomentumVector, corsika::geometry::Point,
                           corsika::units::si::TimeType, unsigned short, unsigned short>&
              v) {
        const unsigned short A = std::get<5>(v);
        const unsigned short Z = std::get<6>(v);
        if (std::get<0>(v) != corsika::particles::Code::Nucleus || A == 0 || Z == 0) {
          std::ostringstream err;
          err << "NuclearStackExtension: no A and Z specified for new Nucleus!";
          throw std::runtime_error(err.str());
        }
        SetNucleusRef(GetStackData().GetNucleusNextRef()); // store this nucleus data ref
        SetNuclearA(A);
        SetNuclearZ(Z);
        InnerParticleInterface<StackIteratorInterface>::SetParticleData(
            p, std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                          corsika::stack::MomentumVector, corsika::geometry::Point,
                          corsika::units::si::TimeType>{std::get<0>(v), std::get<1>(v),
                                                        std::get<2>(v), std::get<3>(v),
                                                        std::get<4>(v)});
      }

      /**
       * @name individual setters
       * @{
       */
      void SetNuclearA(const unsigned short vA) {
        GetStackData().SetNuclearA(GetIndex(), vA);
      }
      void SetNuclearZ(const unsigned short vZ) {
        GetStackData().SetNuclearZ(GetIndex(), vZ);
      }
      /// @}

      /**
       * @name individual getters
       * @{
       */
      int GetNuclearA() const { return GetStackData().GetNuclearA(GetIndex()); }
      int GetNuclearZ() const { return GetStackData().GetNuclearZ(GetIndex()); }
      /// @}

      /**
       * Overwrite normal GetParticleMass function with nuclear version
       */
      corsika::units::si::HEPMassType GetMass() const {
        if (InnerParticleInterface<StackIteratorInterface>::GetPID() ==
            corsika::particles::Code::Nucleus)
          return corsika::particles::GetNucleusMass(GetNuclearA(), GetNuclearZ());
        return InnerParticleInterface<StackIteratorInterface>::GetMass();
      }
      /**
       * Overwirte normal GetChargeNumber function with nuclear version
       **/
      int16_t GetChargeNumber() const {
        if (InnerParticleInterface<StackIteratorInterface>::GetPID() ==
            corsika::particles::Code::Nucleus)
          return GetNuclearZ();
        return InnerParticleInterface<StackIteratorInterface>::GetChargeNumber();
      }

      int GetNucleusRef() const { return GetStackData().GetNucleusRef(GetIndex()); }

    protected:
      void SetNucleusRef(const int vR) { GetStackData().SetNucleusRef(GetIndex(), vR); }
    };

    /**
     * @class NuclearStackExtension
     *
     * Memory implementation of adding nuclear inforamtion to the
     * existing particle stack defined in class InnerStackImpl.
     *
     * Inside the NuclearStackExtension class there is a dedicated
     * fNucleusRef index, where fNucleusRef[i] is referring to the
     * correct A and Z for a specific particle index i. fNucleusRef[i]
     * == -1 means that this is not a nucleus, and a subsequent call to
     * GetNucleusA would produce an exception.
     */
    template <typename InnerStackImpl>
    class NuclearStackExtensionImpl : public InnerStackImpl {

    public:
      void Init() { InnerStackImpl::Init(); }
      void Dump() { InnerStackImpl::Dump(); }

      void Clear() {
        InnerStackImpl::Clear();
        fNucleusRef.clear();
        fNuclearA.clear();
        fNuclearZ.clear();
      }

      unsigned int GetSize() const { return fNucleusRef.size(); }
      unsigned int GetCapacity() const { return fNucleusRef.capacity(); }

      void SetNuclearA(const unsigned int i, const unsigned short vA) {
        fNuclearA[GetNucleusRef(i)] = vA;
      }
      void SetNuclearZ(const unsigned int i, const unsigned short vZ) {
        fNuclearZ[GetNucleusRef(i)] = vZ;
      }
      void SetNucleusRef(const unsigned int i, const int v) { fNucleusRef[i] = v; }

      int GetNuclearA(const unsigned int i) const { return fNuclearA[GetNucleusRef(i)]; }
      int GetNuclearZ(const unsigned int i) const { return fNuclearZ[GetNucleusRef(i)]; }
      // this function will create new storage for Nuclear Properties, and return the
      // reference to it
      int GetNucleusNextRef() {
        fNuclearA.push_back(0);
        fNuclearZ.push_back(0);
        return fNuclearA.size() - 1;
      }

      int GetNucleusRef(const unsigned int i) const {
        if (fNucleusRef[i] >= 0) return fNucleusRef[i];
        std::ostringstream err;
        err << "NuclearStackExtension: no nucleus at ref=" << i;
        throw std::runtime_error(err.str());
      }

      /**
       *   Function to copy particle at location i1 in stack to i2
       */
      void Copy(const unsigned int i1, const unsigned int i2) {
        // index range check
        if (i1 >= GetSize() || i2 >= GetSize()) {
          std::ostringstream err;
          err << "NuclearStackExtension: trying to access data beyond size of stack!";
          throw std::runtime_error(err.str());
        }
        // copy internal particle data p[i2] = p[i1]
        InnerStackImpl::Copy(i1, i2);
        // check if any of p[i1] or p[i2] was a Code::Nucleus
        const int ref1 = fNucleusRef[i1];
        const int ref2 = fNucleusRef[i2];
        if (ref2 < 0) {
          if (ref1 >= 0) {
            // i1 is nucleus, i2 is not
            fNucleusRef[i2] = GetNucleusNextRef();
            fNuclearA[fNucleusRef[i2]] = fNuclearA[ref1];
            fNuclearZ[fNucleusRef[i2]] = fNuclearZ[ref1];
          } else {
            // neither i1 nor i2 are nuclei
          }
        } else {
          if (ref1 >= 0) {
            // both are nuclei, i2 is overwritten with nucleus i1
            // fNucleusRef stays the same, but A and Z data is overwritten
            fNuclearA[ref2] = fNuclearA[ref1];
            fNuclearZ[ref2] = fNuclearZ[ref1];
          } else {
            // i2 is overwritten with non-nucleus i1
            fNucleusRef[i2] = -1;                       // flag as non-nucleus
            fNuclearA.erase(fNuclearA.cbegin() + ref2); // remove data for i2
            fNuclearZ.erase(fNuclearZ.cbegin() + ref2); // remove data for i2
            const int n = fNucleusRef.size(); // update fNucleusRef: indices above ref2
                                              // must be decremented by 1
            for (int i = 0; i < n; ++i) {
              if (fNucleusRef[i] > ref2) { fNucleusRef[i] -= 1; }
            }
          }
        }
      }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Swap(const unsigned int i1, const unsigned int i2) {
        // index range check
        if (i1 >= GetSize() || i2 >= GetSize()) {
          std::ostringstream err;
          err << "NuclearStackExtension: trying to access data beyond size of stack!";
          throw std::runtime_error(err.str());
        }
        // swap original particle data
        InnerStackImpl::Swap(i1, i2);
        // swap corresponding nuclear reference data
        std::swap(fNucleusRef[i2], fNucleusRef[i1]);
      }

      void IncrementSize() {
        InnerStackImpl::IncrementSize();
        fNucleusRef.push_back(-1);
      }

      void DecrementSize() {
        InnerStackImpl::DecrementSize();
        if (fNucleusRef.size() > 0) {
          const int ref = fNucleusRef.back();
          fNucleusRef.pop_back();
          if (ref >= 0) {
            fNuclearA.erase(fNuclearA.begin() + ref);
            fNuclearZ.erase(fNuclearZ.begin() + ref);
            const int n = fNucleusRef.size();
            for (int i = 0; i < n; ++i) {
              if (fNucleusRef[i] >= ref) { fNucleusRef[i] -= 1; }
            }
          }
        }
      }

    private:
      /// the actual memory to store particle data

      std::vector<int> fNucleusRef;
      std::vector<unsigned short> fNuclearA;
      std::vector<unsigned short> fNuclearZ;

    }; // end class NuclearStackExtensionImpl

    //    template<typename StackIteratorInterface>
    // using NuclearParticleInterfaceType<StackIteratorInterface> =
    // NuclearParticleInterface< ,StackIteratorInterface>

    // works, but requires stupd _PI class
    // template<typename SS> using TEST =
    // NuclearParticleInterface<corsika::stack::super_stupid::SuperStupidStack::PIType,
    // SS>;
    template <typename InnerStack, template <typename> typename _PI>
    using NuclearStackExtension =
        Stack<NuclearStackExtensionImpl<typename InnerStack::StackImpl>, _PI>;

    // ----

    // I'm dont't manage to do this properly.......
    /*
    template<typename TT, typename SS> using TESTi = typename
    NuclearParticleInterface<TT::template PIType, SS>::ExtendedParticleInterface;
    template<typename TT, typename SS> using TEST1 = TESTi<TT, SS>;
    template<typename SS> using TEST2 = TEST1<typename
    corsika::stack::super_stupid::SuperStupidStack, SS>;

    using NuclearStackExtension = Stack<NuclearStackExtensionImpl<typename
    InnerStack::StackImpl>, TEST2>;
    */
    /*
      // .... this should be fixed ....

    template <typename InnerStack, typename SS=StackIteratorInterface>
      //using NuclearStackExtension = Stack<NuclearStackExtensionImpl<typename
    InnerStack::StackImpl>, NuclearParticleInterface<typename InnerStack::template PIType,
    StackIteratorInterface>::ExtendedParticleInterface>; using NuclearStackExtension =
    Stack<NuclearStackExtensionImpl<typename InnerStack::StackImpl>, TEST1<typename
    corsika::stack::super_stupid::SuperStupidStack, SS> >;

    //template <typename InnerStack>
      //  using NuclearStackExtension = Stack<NuclearStackExtensionImpl<typename
    InnerStack::StackImpl>, TEST<typename
    corsika::stack::super_stupid::SuperStupidStack::PIType>>;
    //using NuclearStackExtension = Stack<NuclearStackExtensionImpl<typename
    InnerStack::StackImpl>, TEST>;
    */

  } // namespace nuclear_extension
} // namespace corsika::stack

#endif
