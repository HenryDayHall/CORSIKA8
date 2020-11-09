/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  /**
   * \class Line
   *
   * A Line describes a movement in three dimensional space. It
   * consists of a Point `$\vec{p_0}$` and and a speed-Vector
   * `$\vec{v}$`, so that it can return GetPosition as
   * `$\vec{p_0}*\vec{v}*t$` for any value of time `$t$`.
   *
   **/

  class Line {

    using VelocityVec = Vector<corsika::units::si::SpeedType::dimension_type>;

    Point const r0;
    VelocityVec const v0;

  public:
    Line() = delete;
    Line(const Line&) = default;
    Line(Line&&) = default;
    Line& operator=(const Line&) = default;
    Line(Point const& pR0, VelocityVec const& pV0)
        : r0(pR0)
        , v0(pV0) {}

    Point GetPosition(corsika::units::si::TimeType t) const { return r0 + v0 * t; }
    VelocityVec GetVelocity(corsika::units::si::TimeType) const { return v0; }

    Point PositionFromArclength(corsika::units::si::LengthType l) const {
      return r0 + v0.normalized() * l;
    }

    LengthType ArcLength(corsika::units::si::TimeType t1,
                         corsika::units::si::TimeType t2) const {
      return v0.norm() * (t2 - t1);
    }

    corsika::units::si::TimeType TimeFromArclength(
        corsika::units::si::LengthType t) const {
      return t / v0.norm();
    }

    auto GetR0() const { return r0; }
    auto GetV0() const { return v0; }
  };

} // namespace corsika::geometry
