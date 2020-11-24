n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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

    ///! \toto move this to PhysicalUnits
    using VelocityVec = Vector<SpeedType::dimension_type>;

  public:
    Line(Point const& pR0, VelocityVec const& pV0)
        : start_point_(pR0)
        , velocity_(pV0) {}

    inline Point getPosition(TimeType t) const;

    inline Point getPositionFromArclength(LengthType l) const;

    inline LengthType getArcLength(TimeType t1, TimeType t2) const;

    inline TimeType getTimeFromArclength(LengthType t) const;

    inline const Point& getStartPoint() const;

    inline const VelocityVec& getVelocity() const;

  private:
    Point const start_point_;
    VelocityVec const velocity_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Line.inl>
