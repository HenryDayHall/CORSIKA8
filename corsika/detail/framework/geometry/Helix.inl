/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <cmath>

namespace corsika {

  LengthType Helix::getRadius() const { return radius_; }

  Point Helix::getPosition(TimeType const t) const {
    return r0_ + vPar_ * t +
           (vPerp_ * (std::cos(omegaC_ * t) - 1) + uPerp_ * std::sin(omegaC_ * t)) /
               omegaC_;
  }
  inline VelocityVector Helix::getVelocity(TimeType const t) const {
    return vPar_ + (vPerp_ * (cos(omegaC_ * t) - 1) + uPerp_ * sin(omegaC_ * t));
  }

  inline  Point Helix::getPositionFromArclength(LengthType const l) const {
    return getPosition(getTimeFromArclength(l));
  }
  
  inline  LengthType Helix::getArcLength(TimeType const t1, TimeType const t2) const {
    return (vPar_ + vPerp_).getNorm() * (t2 - t1);
  }

  inline  TimeType Helix::getTimeFromArclength(LengthType const l) const {
    return l / (vPar_ + vPerp_).getNorm();
  }

} // namespace corsika
