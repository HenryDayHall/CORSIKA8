/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Volume.hpp>

namespace corsika {

  class Sphere : public Volume {
    Point const fCenter;
    units::si::LengthType const fRadius;

  public:
    Sphere(Point const& pCenter, units::si::LengthType const pRadius)
        : fCenter(pCenter)
        , fRadius(pRadius) {}

    //! returns true if the Point p is within the sphere
    inline bool Contains(Point const& p) const override;

    inline const Point& GetCenter() const;

    inline units::si::LengthType GetRadius() const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Sphere.inl>
