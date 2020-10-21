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

  Point Helix::GetPosition(TimeType t) const {
    return r0 + vPar * t +
           (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
  }

  Point Helix::PositionFromArclength(LengthType l) const {
    return GetPosition(TimeFromArclength(l));
  }

  LengthType Helix::GetRadius() const { return radius; }

  LengthType Helix::ArcLength(TimeType t1, TimeType t2) const {
    return (vPar + vPerp).norm() * (t2 - t1);
  }

  TimeType Helix::TimeFromArclength(LengthType l) const {
    return l / (vPar + vPerp).norm();
  }

} // namespace corsika
