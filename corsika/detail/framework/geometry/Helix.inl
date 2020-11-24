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
#include <cmath>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  LengthType Helix::getRadius() const { return radius_; }

  Point Helix::getPosition(TimeType t) const {
    return r0_ + vPar_ * t +
           (vPerp_ * (std::cos(omegaC_ * t) - 1) + uPerp_ * std::sin(omegaC_ * t)) /
               omegaC_;
  }

  Point Helix::getPositionFromArclength(LengthType l) const {
    return getPosition(getTimeFromArclength(l));
  }

  LengthType Helix::getArcLength(TimeType t1, TimeType t2) const {
    return (vPar_ + vPerp_).norm() * (t2 - t1);
  }

  TimeType Helix::getTimeFromArclength(LengthType l) const {
    return l / (vPar_ + vPerp_).norm();
  }

} // namespace corsika
