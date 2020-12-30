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
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/IVolume.hpp>

namespace corsika {

  /**
   * Describes a sphere in space
   *
   **/

  class Sphere : public IVolume {

  public:
    Sphere(Point const& pCenter, LengthType const pRadius)
        : center_(pCenter)
        , radius_(pRadius) {}

    //! returns true if the Point p is within the sphere
    bool isInside(Point const& p) const override;

    Point const& getCenter() const;

    void setCenter(Point const&);

    LengthType getRadius() const;

    void setRadius(LengthType const);
    
  private:
    Point center_;
    LengthType radius_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Sphere.inl>
