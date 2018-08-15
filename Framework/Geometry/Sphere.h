#ifndef _include_SPHERE_H_
#define _include_SPHERE_H_

#include <Geometry/Point.h>
#include <Units/PhysicalUnits.h>

class Sphere
{
    Point center;
    Length const radius;
    
public:
    Sphere(Point const& pCenter, Length const pRadius) :
        center(pCenter), radius(pRadius)
    {
        
    }
        
    auto isInside(Point const& p) const
    {
        return radius*radius > (center - p).squaredNorm();
    }

};

#endif
