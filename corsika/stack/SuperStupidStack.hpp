/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/stack/Stack.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp> // remove
#include <corsika/framework/geometry/Vector.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace corsika {

  typedef corsika::Vector<hepmomentum_d> MomentumVector;

  namespace super_stupid {

    /**
     * Example of a particle object on the stack.
     */

    template <typename StackIteratorInterface>
    class ParticleInterface : public ParticleBase<StackIteratorInterface> {

    protected:
      using corsika::ParticleBase<StackIteratorInterface>::GetStack;
      using corsika::ParticleBase<StackIteratorInterface>::GetStackData;

    public:
      using corsika::ParticleBase<StackIteratorInterface>::GetIndex;

    public:
      void SetParticleData(const std::tuple<corsika::Code, HEPEnergyType, MomentumVector,
                                            corsika::Point, TimeType>& v) {
        SetPID(std::get<0>(v));
        SetEnergy(std::get<1>(v));
        SetMomentum(std::get<2>(v));
        SetPosition(std::get<3>(v));
        SetTime(std::get<4>(v));
      }
      /*
    void SetParticleData(const corsika::Code vDataPID,
                         const HEPEnergyType vDataE,
                         const MomentumVector& vMomentum,
                         const corsika::Point& vPosition,
                         const TimeType vTime) {
      }*/

      void SetParticleData(ParticleInterface<StackIteratorInterface>&,
                           const std::tuple<corsika::Code, HEPEnergyType, MomentumVector,
                                            corsika::Point, TimeType>& v) {
        SetPID(std::get<0>(v));
        SetEnergy(std::get<1>(v));
        SetMomentum(std::get<2>(v));
        SetPosition(std::get<3>(v));
        SetTime(std::get<4>(v));
      }
      /*      void SetParticleData(ParticleInterface<StackIteratorInterface>&,
                           const corsika::Code vDataPID,
                           const HEPEnergyType vDataE,
                           const MomentumVector& vMomentum,
                           const corsika::Point& vPosition,
                           const TimeType vTime) {
        SetPID(vDataPID);
        SetEnergy(vDataE);
        SetMomentum(vMomentum);
        SetPosition(vPosition);
        SetTime(vTime);
      }*/

      /// individual setters
      void SetPID(const corsika::Code id) { GetStackData().SetPID(GetIndex(), id); }
      void SetEnergy(const HEPEnergyType& e) { GetStackData().SetEnergy(GetIndex(), e); }
      void SetMomentum(const MomentumVector& v) {
        GetStackData().SetMomentum(GetIndex(), v);
      }
      void SetPosition(const corsika::Point& v) {
        GetStackData().SetPosition(GetIndex(), v);
      }
      void SetTime(const TimeType& v) { GetStackData().SetTime(GetIndex(), v); }

      /// individual getters
      corsika::Code GetPID() const { return GetStackData().GetPID(GetIndex()); }
      HEPEnergyType GetEnergy() const { return GetStackData().GetEnergy(GetIndex()); }
      MomentumVector GetMomentum() const {
        return GetStackData().GetMomentum(GetIndex());
      }
      corsika::Point GetPosition() const {
        return GetStackData().GetPosition(GetIndex());
      }
      TimeType GetTime() const { return GetStackData().GetTime(GetIndex()); }
      /**
       * @name derived quantities
       *
       * @{
       */
      corsika::Vector<dimensionless_d> GetDirection() const {
        return GetMomentum() / GetEnergy();
      }
      HEPMassType GetMass() const { return corsika::GetMass(GetPID()); }
      int16_t GetChargeNumber() const { return corsika::GetChargeNumber(GetPID()); }
      ///@}
    };

    /**
     * Memory implementation of the most simple (stupid) particle stack object.
     */

    class SuperStupidStackImpl {

    public:
      void Init() {}
      void Dump() const {}

      void Clear() {
        fDataPID.clear();
        fDataE.clear();
        fMomentum.clear();
        fPosition.clear();
        fTime.clear();
      }

      unsigned int GetSize() const { return fDataPID.size(); }
      unsigned int GetCapacity() const { return fDataPID.size(); }

      void SetPID(const unsigned int i, const corsika::Code id) { fDataPID[i] = id; }
      void SetEnergy(const unsigned int i, const HEPEnergyType e) { fDataE[i] = e; }
      void SetMomentum(const unsigned int i, const MomentumVector& v) {
        fMomentum[i] = v;
      }
      void SetPosition(const unsigned int i, const corsika::Point& v) {
        fPosition[i] = v;
      }
      void SetTime(const unsigned int i, const TimeType& v) { fTime[i] = v; }

      corsika::Code GetPID(const unsigned int i) const { return fDataPID[i]; }
      HEPEnergyType GetEnergy(const unsigned int i) const { return fDataE[i]; }
      MomentumVector GetMomentum(const unsigned int i) const { return fMomentum[i]; }
      corsika::Point GetPosition(const unsigned int i) const { return fPosition[i]; }
      TimeType GetTime(const unsigned int i) const { return fTime[i]; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Copy(const unsigned int i1, const unsigned int i2) {
        fDataPID[i2] = fDataPID[i1];
        fDataE[i2] = fDataE[i1];
        fMomentum[i2] = fMomentum[i1];
        fPosition[i2] = fPosition[i1];
        fTime[i2] = fTime[i1];
      }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Swap(const unsigned int i1, const unsigned int i2) {
        std::swap(fDataPID[i2], fDataPID[i1]);
        std::swap(fDataE[i2], fDataE[i1]);
        std::swap(fMomentum[i2], fMomentum[i1]);
        std::swap(fPosition[i2], fPosition[i1]);
        std::swap(fTime[i2], fTime[i1]);
      }

      void IncrementSize() {
        using corsika::Code;
        using corsika::Point;
        fDataPID.push_back(Code::Unknown);
        fDataE.push_back(0 * electronvolt);
        CoordinateSystem& dummyCS =
            RootCoordinateSystem::getInstance().GetRootCoordinateSystem();
        fMomentum.push_back(MomentumVector(
            dummyCS, {0 * electronvolt, 0 * electronvolt, 0 * electronvolt}));
        fPosition.push_back(Point(dummyCS, {0 * meter, 0 * meter, 0 * meter}));
        fTime.push_back(0 * second);
      }

      void DecrementSize() {
        if (fDataE.size() > 0) {
          fDataPID.pop_back();
          fDataE.pop_back();
          fMomentum.pop_back();
          fPosition.pop_back();
          fTime.pop_back();
        }
      }

    private:
      /// the actual memory to store particle data

      std::vector<corsika::Code> fDataPID;
      std::vector<HEPEnergyType> fDataE;
      std::vector<MomentumVector> fMomentum;
      std::vector<corsika::Point> fPosition;
      std::vector<TimeType> fTime;

    }; // end class SuperStupidStackImpl

    typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;

  } // namespace super_stupid

} // namespace corsika

