n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/IVolume.hpp>

namespace corsika {

  class Sphere : public IVolume {

  public:
    Sphere(Point const& pCenter, LengthType const pRadius)
        : center_(pCenter)
        , radius_(pRadius) {}

    //! returns true if the Point p is within the sphere
    inline bool isInside(Point const& p) const override;

    inline const Point& getCenter() const;

    inline LengthType getRadius() const;

  private:
    Point const center_;
    LengthType const radius_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Sphere.inl>
