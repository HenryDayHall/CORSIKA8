
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

    using corsika::geometry::Point;
    using corsika::geometry::Vector;
    using corsika::particles::Code;
    using corsika::units::si::energy_d;
    using corsika::units::si::EnergyType;
    using corsika::units::si::joule;
    using corsika::units::si::meter;
    using corsika::units::si::momentum_d;
    using corsika::units::si::newton_second;
    using corsika::units::si::second;
    using corsika::units::si::SpeedType;
    using corsika::units::si::TimeType;

    typedef Vector<momentum_d> MomentumVector;

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
      void SetMomentum(const MomentumVector& v) {
        GetStackData().SetMomentum(GetIndex(), v);
      }
      void SetPosition(const Point& v) { GetStackData().SetPosition(GetIndex(), v); }
      void SetTime(const TimeType& v) { GetStackData().SetTime(GetIndex(), v); }

      Code GetPID() const { return GetStackData().GetPID(GetIndex()); }
      EnergyType GetEnergy() const { return GetStackData().GetEnergy(GetIndex()); }
      MomentumVector GetMomentum() const {
        return GetStackData().GetMomentum(GetIndex());
      }
      Point GetPosition() const { return GetStackData().GetPosition(GetIndex()); }
      TimeType GetTime() const { return GetStackData().GetTime(GetIndex()); }
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

      void SetPID(const int i, const Code id) { fDataPID[i] = id; }
      void SetEnergy(const int i, const EnergyType e) { fDataE[i] = e; }
      void SetMomentum(const int i, const MomentumVector& v) { fMomentum[i] = v; }
      void SetPosition(const int i, const Point& v) { fPosition[i] = v; }
      void SetTime(const int i, const TimeType& v) { fTime[i] = v; }

      Code GetPID(const int i) const { return fDataPID[i]; }
      EnergyType GetEnergy(const int i) const { return fDataE[i]; }
      MomentumVector GetMomentum(const int i) const { return fMomentum[i]; }
      Point GetPosition(const int i) const { return fPosition[i]; }
      TimeType GetTime(const int i) const { return fTime[i]; }

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
        fDataPID.push_back(Code::Unknown);
        fDataE.push_back(0 * joule);
        //#TODO this here makes no sense: see issue #48
        geometry::CoordinateSystem& dummyCS =
            geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
        fMomentum.push_back(MomentumVector(
            dummyCS, {0 * newton_second, 0 * newton_second, 0 * newton_second}));
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

      std::vector<Code> fDataPID;
      std::vector<EnergyType> fDataE;
      std::vector<MomentumVector> fMomentum;
      std::vector<Point> fPosition;
      std::vector<TimeType> fTime;

    }; // end class SuperStupidStackImpl

    typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;

  } // namespace super_stupid

} // namespace corsika::stack

#endif
