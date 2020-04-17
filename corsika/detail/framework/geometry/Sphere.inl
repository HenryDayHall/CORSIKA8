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
#include <corsika/framework/geometry/Volume.hpp>

namespace corsika {

    //! returns true if the Point p is within the sphere
    bool Sphere::Contains(Point const& p) const
    {
      return fRadius * fRadius > (fCenter - p).squaredNorm();
    }

    const Point& Sphere::GetCenter() const
    {
    	return fCenter;
    }

    units::si::LengthType Sphere::GetRadius() const
    {
    	return fRadius;
    }

} // namespace corsika

