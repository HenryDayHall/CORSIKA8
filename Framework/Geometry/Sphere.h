
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_SPHERE_H_
#define _include_SPHERE_H_

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  class Sphere {
    Point center;
    LengthType const radius;

  public:
    Sphere(Point const& pCenter, LengthType const pRadius)
        : center(pCenter)
        , radius(pRadius) {}

    //! returns true if the Point \a p is within the sphere
    auto Contains(Point const& p) const {
      return radius * radius > (center - p).squaredNorm();
    }
  };

} // namespace corsika::geometry

#endif
