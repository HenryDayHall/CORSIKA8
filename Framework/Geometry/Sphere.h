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

    auto isInside(Point const& p) const {
      return radius * radius > (center - p).squaredNorm();
    }
  };

} // namespace corsika::geometry

#endif
