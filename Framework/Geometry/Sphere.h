#ifndef _include_SPHERE_H_
#define _include_SPHERE_H_

#include <corsika/geometry/Volume.h>
#include <corsika/geometry/Point.h>
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
  };

} // namespace corsika::geometry

#endif
