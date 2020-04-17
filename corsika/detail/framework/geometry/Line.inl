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


    Point Line::GetPosition(units::si::TimeType t) const
    {
    	return r0 + v0 * t;
    }

    Point Line::PositionFromArclength(units::si::LengthType l) const
    {
      return r0 + v0.normalized() * l;
    }

    units::si::LengthType
	Line::ArcLength(units::si::TimeType t1, units::si::TimeType t2) const
    {
      return v0.norm() * (t2 - t1);
    }

    units::si::TimeType
	Line::TimeFromArclength( units::si::LengthType t) const
    {
      return t / v0.norm();
    }

    const Point& Line::GetR0() const
    {
    	return r0;
    }

    const Line::VelocityVec& Line::GetV0() const
    {
    	return v0;
    }


} // namespace corsika

