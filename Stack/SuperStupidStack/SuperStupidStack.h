
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

#include <corsika/geometry/RootCoordinateSystem.h> // remove 
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>

#include <vector>
#include <algorithm>

using namespace corsika;

namespace corsika::stack {

  namespace super_stupid {

    using particles::Code;
    using units::si::EnergyType;
    using units::si::TimeType;
    using units::si::SpeedType;
    using units::si::second;
    using units::si::meter;
    using units::si::joule;
    using units::si::energy_d;
    using geometry::Point;
    using geometry::Vector;

#warning replace this with a proper momentum vector:
    typedef Vector<energy_d> MomentumVector; // should be momentum_d !!!

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
      void SetMomentum(const MomentumVector& v) { GetStackData().SetMomentum(GetIndex(), v); }
      void SetPosition(const Point& v) { GetStackData().SetPosition(GetIndex(), v); }
      void SetTime(const TimeType& v) { GetStackData().SetTime(GetIndex(), v); }

      Code GetPID() const { return GetStackData().GetPID(GetIndex()); }
      EnergyType GetEnergy() const { return GetStackData().GetEnergy(GetIndex()); }
      MomentumVector GetMomentum() const { return GetStackData().GetMomentum(GetIndex()); }
      Point GetPosition() const { return GetStackData().GetPosition(GetIndex()); }
      TimeType GetTime() const { return GetStackData().GetTime(GetIndex()); }
      
#warning this does not really work, nor make sense:
      Vector<SpeedType::dimension_type> GetDirection() const {
	auto P = GetMomentum();
	return P/P.norm() * (units::si::meter/units::si::second); }
    
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
	std::swap(fMomentum[i2], fMomentum[i1]); // should be Momentum !!!!
	std::swap(fPosition[i2], fPosition[i1]);
	std::swap(fTime[i2], fTime[i1]);
      }

    protected:
      void IncrementSize() {
        fDataPID.push_back(Code::Unknown);
        fDataE.push_back(0 * joule);
#warning this here makes no sense: see issue #48
	geometry::CoordinateSystem& dummyCS = geometry::RootCoordinateSystem::GetInstance().GetRootCS(); 
	fMomentum.push_back(MomentumVector(dummyCS,
					   {0 * joule, 0 * joule, 0 * joule}));	
	fPosition.push_back(Point(dummyCS,
				  {0 * meter, 0 * meter, 0 * meter}));
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
      std::vector<Vector<units::si::energy_d>> fMomentum; // should be Momentum !!!!
      std::vector<Point> fPosition;
      std::vector<TimeType> fTime;

    }; // end class SuperStupidStackImpl

    typedef Stack<SuperStupidStackImpl, ParticleInterface> SuperStupidStack;

  } // namespace super_stupid

} // namespace corsika::stack

#endif
