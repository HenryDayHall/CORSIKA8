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


    Point Helix::GetPosition(corsika::units::si::TimeType t) const
    {
      return r0 + vPar * t +
             (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
    }

    Point Helix::PositionFromArclength(corsika::units::si::LengthType l) const
    {
      return GetPosition(TimeFromArclength(l));
    }

    units::si::LengthType Helix::GetRadius() const
    {
    	return radius;
    }

    corsika::units::si::LengthType
    Helix::ArcLength(corsika::units::si::TimeType t1, corsika::units::si::TimeType t2) const
    {
      return (vPar + vPerp).norm() * (t2 - t1);
    }

    corsika::units::si::TimeType
	Helix::TimeFromArclength(corsika::units::si::LengthType l) const
    {
      return l / (vPar + vPerp).norm();
    }


} // namespace corsika

