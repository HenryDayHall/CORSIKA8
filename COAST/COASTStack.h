/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_coaststack_h_
#define _include_coaststack_h_

#include <corsika/coast/ParticleConversion.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/stack/ParticleBase.h>
#include <corsika/stack/Stack.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h> // remove
#include <corsika/geometry/Vector.h>

#include <crs/CParticle.h>
#include <crs/CorsikaTypes.h>

#include <algorithm>
#include <vector>

namespace corsika::coast {

  typedef corsika::geometry::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

  /**
   * Example of a particle object on the stack.
   */

  template <typename StackIteratorInterface>
  class ParticleInterface : public corsika::stack::ParticleBase<StackIteratorInterface> {

    using corsika::stack::ParticleBase<StackIteratorInterface>::GetStackData;
    using corsika::stack::ParticleBase<StackIteratorInterface>::GetIndex;

  public:
    corsika::particles::Code GetPID() const { return GetStackData().GetPID(GetIndex()); }

    corsika::units::si::HEPEnergyType GetEnergy() const {
      return GetStackData().GetEnergy(GetIndex());
    }

    MomentumVector GetMomentum(const corsika::geometry::CoordinateSystem& cs) const {
      using namespace corsika::units::si;
      const HEPEnergyType mass = corsika::particles::GetMass(GetPID());
      const auto P = sqrt((GetEnergy() - mass) * (GetEnergy() + mass));
      const auto p = GetStackData().GetDirection(cs);
      return p * P;
    }

    corsika::geometry::Point GetPosition(
        const corsika::geometry::CoordinateSystem& cs) const {
      return GetStackData().GetPosition(cs, GetIndex());
    }

    corsika::geometry::Vector<corsika::units::si::speed_d> GetVelocity(
        const corsika::geometry::CoordinateSystem& cs) const {
      return GetStackData().GetVelocity(cs, GetIndex());
    }

    corsika::units::si::TimeType GetTime() const {
      return GetStackData().GetTime(GetIndex());
    }

    corsika::geometry::Vector<corsika::units::si::dimensionless_d> GetDirection(
        const corsika::geometry::CoordinateSystem& cs) const {
      return GetStackData().GetDirection(cs);
    }

    corsika::units::si::TimeType GetTimeInterval() const {
      return GetStackData().GetTimeInterval();
    }
  };

  /**
   *
   * Memory implementation of the most simple (stupid) particle stack object.
   */

  class COASTStackImpl {

    const crs::CParticle* fParticle1 = 0;
    const crs::CParticle* fParticle2 = 0;

  public:
    COASTStackImpl(const crs::CParticle* v1, const crs::CParticle* v2) {
      fParticle1 = v1;
      fParticle2 = v2;
    }

    void Init() {}
    void Clear() {}

    // there is one particle only
    int GetSize() const { return 1; }
    int GetCapacity() const { return 1; }

    // you cannot modify the particle:
    // there are no Set... function defined

    // readout particle data, there is just one particle!
    /*
      double x;
      double y;
      double z;
      double depth;
      double time;
      double energy;
      double weight;
      int    particleId;
      int    hadronicGeneration;
    */
    corsika::particles::Code GetPID(const int) const {
      return ConvertFromCoast(static_cast<CoastCode>(fParticle1->particleId));
    }
    corsika::units::si::HEPEnergyType GetEnergy(const int) const {
      using namespace corsika::units::si;
      return fParticle1->energy * 1_GeV;
    }
    corsika::geometry::Vector<corsika::units::si::dimensionless_d> GetDirection(
        const corsika::geometry::CoordinateSystem& cs) const {
      using namespace corsika::units::si;
      corsika::geometry::Point p1(
          cs, {fParticle1->x * 1_cm, fParticle1->y * 1_cm, fParticle1->z * 1_cm});
      corsika::geometry::Point p2(
          cs, {fParticle2->x * 1_cm, fParticle2->y * 1_cm, fParticle2->z * 1_cm});
      const corsika::geometry::Vector D = p2 - p1;
      const auto magD = D.norm();
      const corsika::geometry::Vector dir = D / magD;
      return dir;
    }
    corsika::geometry::Vector<corsika::units::si::speed_d> GetVelocity(
        const corsika::geometry::CoordinateSystem& cs, const int) const {
      using namespace corsika::units::si;
      corsika::geometry::Vector<corsika::units::si::dimensionless_d> dir =
          GetDirection(cs);
      corsika::geometry::Point p1(
          cs, {fParticle1->x * 1_cm, fParticle1->y * 1_cm, fParticle1->z * 1_cm});
      corsika::geometry::Point p2(
          cs, {fParticle2->x * 1_cm, fParticle2->y * 1_cm, fParticle2->z * 1_cm});
      const corsika::geometry::Vector D = p2 - p1;
      const LengthType magD = D.norm();
      const TimeType deltaT = GetTimeInterval();
      return dir * magD / deltaT;
    }
    corsika::geometry::Point GetPosition(const corsika::geometry::CoordinateSystem& cs,
                                         const int) const {
      using namespace corsika::units::si;
      return corsika::geometry::Point(
          cs, {fParticle1->x * 1_cm, fParticle1->y * 1_cm, fParticle1->z * 1_cm});
    }
    corsika::units::si::TimeType GetTime(const int) const {
      using namespace corsika::units::si;
      return fParticle1->time * 1_s;
    }

    corsika::units::si::TimeType GetTimeInterval() const {
      using namespace corsika::units::si;
      return (fParticle2->time - fParticle1->time) * 1_s;
    }

    /**
     *   We do not allow copying!
     */
    void Copy(const int, const int) {}

    /**
     *   We do not allow swapping particles, there is just one...
     */
    void Swap(const int, const int) {}

    // size cannot be increased, do nothing
    void IncrementSize() {}

    // size cannot be decremented, do nothing
    void DecrementSize() {}

  }; // end class COASTStackImpl

  typedef corsika::stack::Stack<COASTStackImpl, ParticleInterface> COASTStack;

} // namespace corsika::coast

#endif
