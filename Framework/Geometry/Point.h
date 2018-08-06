#ifndef _include_POINT_H_
#define _include_POINT_H_

#include <Geometry/BaseVector.h>
#include <Geometry/QuantityVector.h>
#include <Units/PhysicalUnits.h>

class Point : public BaseVector<phys::units::length_d>
{
    using Length = phys::units::quantity<phys::units::length_d, double>;
    
    
public:
    Point(CoordinateSystem const& pCS, QuantityVector<phys::units::length_d> pQVector) :
        BaseVector<phys::units::length_d>(pCS, pQVector)
    {
    }

    Point(CoordinateSystem const& cs, Length x, Length y, Length z) :
        BaseVector<phys::units::length_d>(cs, {x, y, z})
    {
    }
    
    auto getCoordinates() const
    {
        return BaseVector<phys::units::length_d>::qVector;
    }
    
    auto getCoordinates(CoordinateSystem const& pCS) const
    {
        if (&pCS == BaseVector<phys::units::length_d>::cs)
        {
            return BaseVector<phys::units::length_d>::qVector;
        }
        else
        {
            return QuantityVector<phys::units::length_d>(CoordinateSystem::getTransformation(*BaseVector<phys::units::length_d>::cs, pCS) * BaseVector<phys::units::length_d>::qVector.eVector);
        }
    }
    
    void rebase(CoordinateSystem const& pCS)
    {
        BaseVector<phys::units::length_d>::qVector = getCoordinates(pCS);
        BaseVector<phys::units::length_d>::cs = &pCS;
    }
};

#endif
