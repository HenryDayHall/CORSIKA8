#ifndef SPHERE_H_
#define SPHERE_H_

#include "Point.h"
#include <phys/units/quantity.hpp>

class Sphere
{
    using Length = phys::units::quantity<phys::units::length_d, double>;
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
