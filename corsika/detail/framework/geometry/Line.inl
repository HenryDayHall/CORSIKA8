/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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

  Point Line::getPosition(TimeType t) const { return start_point_ + velocity_ * t; }

  Point Line::getPositionFromArclength(LengthType l) const {
    return start_point_ + velocity_.normalized() * l;
  }

  LengthType Line::getArcLength(TimeType t1, TimeType t2) const {
    return velocity_.getNorm() * (t2 - t1);
  }

  TimeType Line::getTimeFromArclength(LengthType t) const {
    return t / velocity_.getNorm();
  }

  Point const& Line::getStartPoint() const { return start_point_; }

  Line::VelocityVec const& Line::getVelocity() const { return velocity_; }

} // namespace corsika
