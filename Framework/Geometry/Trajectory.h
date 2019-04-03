/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_TRAJECTORY_H
#define _include_TRAJECTORY_H

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  template <typename T>
  class Trajectory : public T {

    corsika::units::si::TimeType fTimeLength;

  public:
    using T::ArcLength;
    using T::GetPosition;

    Trajectory(T const& theT, corsika::units::si::TimeType timeLength)
        : T(theT)
        , fTimeLength(timeLength) {}

    /*Point GetPosition(corsika::units::si::TimeType t) const {
      return fTraj.GetPosition(t + fTStart);
      }*/

    Point GetPosition(double u) const { return T::GetPosition(fTimeLength * u); }

    corsika::units::si::TimeType GetDuration() const { return fTimeLength; }
    corsika::units::si::LengthType GetLength() const { return GetDistance(fTimeLength); }

    corsika::units::si::LengthType GetDistance(corsika::units::si::TimeType t) const {
      assert(t <= fTimeLength);
      assert(t >= 0 * corsika::units::si::second);
      return T::ArcLength(0 * corsika::units::si::second, t);
    }

    void LimitEndTo(corsika::units::si::LengthType limit) {
      fTimeLength = T::TimeFromArclength(limit);
    }

    auto NormalizedDirection() const {
      static_assert(std::is_same_v<T, corsika::geometry::Line>);
      return T::GetV0().normalized();
    }
  };

} // namespace corsika::geometry

#endif
