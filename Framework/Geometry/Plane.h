/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Framework_Geometry_Plane_h_
#define _include_Framework_Geometry_Plane_h_

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {
  class Plane {

    using DimLessVec = Vector<corsika::units::si::dimensionless_d>;

    Point const fCenter;
    DimLessVec const fNormal;

  public:
    Plane(Point const& vCenter, DimLessVec const& vNormal)
        : fCenter(vCenter)
        , fNormal(vNormal) {}
    bool IsAbove(Point const& vP) const {
      return fNormal.dot(vP - fCenter) > corsika::units::si::LengthType::zero();
    }

    Point const& GetCenter() const { return fCenter; }
    DimLessVec const& GetNormal() const { return fNormal; }
  };

} // namespace corsika::geometry

#endif
