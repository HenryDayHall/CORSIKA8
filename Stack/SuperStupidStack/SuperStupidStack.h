
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

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h> // remove
#include <corsika/geometry/Vector.h>

#include <algorithm>
#include <vector>

using namespace corsika;

namespace corsika::stack {

  namespace super_stupid {

    typedef corsika::geometry::Vector<corsika::units::hep::energy_hep_d> MomentumVector;

    /**
     * Example of a particle object on the stack.
     */

    template <typename StackIteratorInterface>
    class ParticleInterface : public ParticleBase<StackIteratorInterface> {

      using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
      using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

    public:
      void SetPID(const corsika::particles::Code id) {
        GetStackData().SetPID(GetIndex(), id);
      }

      void SetEnergy(const corsika::units::hep::EnergyType& e) {
        GetStackData().SetEnergy(GetIndex(), e);
      }

      void SetMomentum(const MomentumVector& v) {
        GetStackData().SetMomentum(GetIndex(), v);
      }

      void SetPosition(const corsika::geometry::Point& v) {
        GetStackData().SetPosition(GetIndex(), v);
      }

      void SetTime(const corsika::units::si::TimeType& v) {
        GetStackData().SetTime(GetIndex(), v);
      }

      corsika::particles::Code GetPID() const {
        return GetStackData().GetPID(GetIndex());
      }

      corsika::units::hep::EnergyType GetEnergy() const {
        return GetStackData().GetEnergy(GetIndex());
      }

      MomentumVector GetMomentum() const {
        return GetStackData().GetMomentum(GetIndex());
      }

      corsika::geometry::Point GetPosition() const {
        return GetStackData().GetPosition(GetIndex());
      }

      corsika::units::si::TimeType GetTime() const {
        return GetStackData().GetTime(GetIndex());
      }

      corsika::geometry::Vector<corsika::units::si::SpeedType::dimension_type>
      GetDirection() const {
        return GetMomentum() / GetEnergy() * corsika::units::constants::c;
      }
    };

    /**
     *
     * Memory implementation of the most simple (stupid) particle stack object.
     */

    class SuperStupidStackImpl {

    public:
      void Init() {}

      void Clear() {
        fDataPID.clear();
        fDataE.clear();
        fMomentum.clear();
        fPosition.clear();
        fTime.clear();
      }

      int GetSize() const { return fDataPID.size(); }

      int GetCapacity() const { return fDataPID.size(); }

      void SetPID(const int i, const corsika::particles::Code id) { fDataPID[i] = id; }

      void SetEnergy(const int i, const corsika::units::hep::EnergyType e) {
        fDataE[i] = e;
      }
      void SetMomentum(const int i, const MomentumVector& v) { fMomentum[i] = v; }

      void SetPosition(const int i, const corsika::geometry::Point& v) {
        fPosition[i] = v;
      }
      
      void SetTime(const int i, const corsika::units::si::TimeType& v) { fTime[i] = v; }

      corsika::particles::Code GetPID(const int i) const { return fDataPID[i]; }

      corsika::units::hep::EnergyType GetEnergy(const int i) const { return fDataE[i]; }

      MomentumVector GetMomentum(const int i) const { return fMomentum[i]; }

      corsika::geometry::Point GetPosition(const int i) const { return fPosition[i]; }

      corsika::units::si::TimeType GetTime(const int i) const { return fTime[i]; }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Copy(const int i1, const int i2) {
        fDataPID[i2] = fDataPID[i1];
        fDataE[i2] = fDataE[i1];
        fMomentum[i2] = fMomentum[i1];
        fPosition[i2] = fPosition[i1];
        fTime[i2] = fTime[i1];
      }

      /**
       *   Function to copy particle at location i2 in stack to i1
       */
      void Swap(const int i1, const int i2) {
        std::swap(fDataPID[i2], fDataPID[i1]);
        std::swap(fDataE[i2], fDataE[i1]);
        std::swap(fMomentum[i2], fMomentum[i1]);
        std::swap(fPosition[i2], fPosition[i1]);
        std::swap(fTime[i2], fTime[i1]);
      }

    protected:
      void IncrementSize() {
        using corsika::geometry::Point;
        using corsika::particles::Code;
        fDataPID.push_back(Code::Unknown);
        fDataE.push_back(0 * corsika::units::si::joule);
        //#TODO this here makes no sense: see issue #48
        geometry::CoordinateSystem& dummyCS =
            geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
        fMomentum.push_back(MomentumVector(
            dummyCS, {0 * corsika::units::si::joule, 0 * corsika::units::si::joule,
                      0 * corsika::units::si::joule}));
        fPosition.push_back(
            Point(dummyCS, {0 * corsika::units::si::meter, 0 * corsika::units::si::meter,
                            0 * corsika::units::si::meter}));
        fTime.push_back(0 * corsika::units::si::second);
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

      std::vector<corsika::particles::Code> fDataPID;
      std::vector<corsika::units::hep::EnergyType> fDataE;
      std::vector<MomentumVector> fMomentum;
      std::vector<corsika::geometry::Point> fPosition;
      std::vector<corsika::units::si::TimeType> fTime;

    }; // end class SuperStupidStackImpl

    typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;

  } // namespace super_stupid

} // namespace corsika::stack

#endif
