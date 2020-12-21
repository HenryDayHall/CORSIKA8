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
   * Describes a straight line in space
   *
   */

  class Line {

    ///! \toto move this to PhysicalUnits
    using VelocityVec = Vector<SpeedType::dimension_type>;

  public:
    Line(Point const& pR0, VelocityVec const& pV0)
        : start_point_(pR0)
        , velocity_(pV0) {}

    inline Point getPosition(TimeType const t) const;

    inline Point getPositionFromArclength(LengthType const l) const;

    inline LengthType getArcLength(TimeType const t1, TimeType const t2) const;

    inline TimeType getTimeFromArclength(LengthType const t) const;

    inline Point const& getStartPoint() const;
    inline Point& startPoint() { return start_point_; }

    inline VelocityVec const& getVelocity() const;
    inline VelocityVec& velocity() { return velocity_; }

  private:
    Point start_point_;
    VelocityVec velocity_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Line.inl>
