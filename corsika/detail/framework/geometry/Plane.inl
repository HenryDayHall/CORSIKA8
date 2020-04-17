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

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {


    bool Plane::IsAbove(Point const& vP) const
    {
      return fNormal.dot(vP - fCenter) > corsika::units::si::LengthType::zero();
    }

    units::si::LengthType Plane::DistanceTo(geometry::Point const& vP) const
    {
      return (fNormal * (vP - fCenter).dot(fNormal)).norm();
    }

    Point const& Plane::GetCenter() const
    {
    	return fCenter;
    }

    Plane::DimLessVec const& Plane::GetNormal() const
    {
    	return fNormal;
    }


} // namespace corsika

