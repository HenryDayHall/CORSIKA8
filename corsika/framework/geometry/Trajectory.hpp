/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  template <typename T>
  class Trajectory : public T {

    TimeType fTimeLength;

  public:
    using T::ArcLength;
    using T::GetPosition;

    Trajectory(T const& theT, TimeType timeLength)
        : T(theT)
        , fTimeLength(timeLength) {}

    /*Point GetPosition(TimeType t) const {
      return fTraj.GetPosition(t + fTStart);
      }*/

    Point GetPosition(double u) const;

    TimeType GetDuration() const;

    LengthType GetLength() const;

    LengthType GetDistance(TimeType t) const;

    void LimitEndTo(LengthType limit);

    auto NormalizedDirection() const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Trajectory.inl>
