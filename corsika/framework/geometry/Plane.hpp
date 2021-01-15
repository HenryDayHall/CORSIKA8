/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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

  class Plane {

    ///! \todo move to PhysicalUnits
    using DimLessVec = Vector<dimensionless_d>;

  public:
    Plane(Point const& vCenter, DimLessVec const& vNormal)
        : center_(vCenter)
        , normal_(vNormal.normalized()) {}

    bool isAbove(Point const& vP) const;

    LengthType getDistanceTo(corsika::Point const& vP) const;

    Point const& getCenter() const;

    DimLessVec const& getNormal() const;

  public:
    Point const center_;
    DimLessVec const normal_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Plane.inl>
