/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  class Plane {

    using DimLessVec = Vector<corsika::units::si::dimensionless_d>;

    Point const fCenter;
    DimLessVec const fNormal;

  public:

    Plane(Point const& vCenter, DimLessVec const& vNormal)
        : fCenter(vCenter)
        , fNormal(vNormal.normalized()) {}

    bool IsAbove(Point const& vP) const ;

    units::si::LengthType DistanceTo(corsika::Point const& vP) const ;


    Point const& GetCenter() const ;

    DimLessVec const& GetNormal() const ;

  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Plane.inl>

