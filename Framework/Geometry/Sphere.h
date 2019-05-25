/*
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
#include <corsika/geometry/Volume.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  class Sphere : public Volume {
    Point const fCenter;
    LengthType const fRadius;

  public:
    Sphere(Point const& pCenter, LengthType const pRadius)
        : fCenter(pCenter)
        , fRadius(pRadius) {}

    //! returns true if the Point p is within the sphere
    bool Contains(Point const& p) const override {
      return fRadius * fRadius > (fCenter - p).squaredNorm();
    }

    auto& GetCenter() const { return fCenter; }
    auto GetRadius() const { return fRadius; }
  };

} // namespace corsika::geometry

#endif
