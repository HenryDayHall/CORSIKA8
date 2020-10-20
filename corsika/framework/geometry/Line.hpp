/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

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

    using VelocityVec = Vector<units::si::SpeedType::dimension_type>;

    Point const r0;
    VelocityVec const v0;

  public:
    Line(Point const& pR0, VelocityVec const& pV0)
        : r0(pR0)
        , v0(pV0) {}

    inline Point GetPosition(units::si::TimeType t) const;

    inline Point PositionFromArclength(units::si::LengthType l) const;

    inline units::si::LengthType ArcLength(units::si::TimeType t1,
                                           units::si::TimeType t2) const;

    inline units::si::TimeType TimeFromArclength(units::si::LengthType t) const;

    inline const Point& GetR0() const;

    inline const VelocityVec& GetV0() const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Line.inl>
