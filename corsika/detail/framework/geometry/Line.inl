/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  Point Line::getPosition(TimeType const t) const { return start_point_ + velocity_ * t; }

  Point Line::getPositionFromArclength(LengthType const l) const {
    return start_point_ + velocity_.normalized() * l;
  }

  LengthType Line::getArcLength(TimeType const t1, TimeType const t2) const {
    return velocity_.getNorm() * (t2 - t1);
  }

  TimeType Line::getTimeFromArclength(LengthType const t) const {
    return t / velocity_.getNorm();
  }

  Point const& Line::getStartPoint() const { return start_point_; }

  Line::VelocityVec const& Line::getVelocity() const { return velocity_; }

} // namespace corsika
