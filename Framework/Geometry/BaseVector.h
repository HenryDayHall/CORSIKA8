#ifndef _include_BASEVECTOR_H_
#define _include_BASEVECTOR_H_

#include <Geometry/QuantityVector.h>
#include <Geometry/CoordinateSystem.h>

template <typename dim>
class BaseVector
{
protected:
    QuantityVector<dim> qVector;
    CoordinateSystem const* cs;
    
public:
    BaseVector(CoordinateSystem const& pCS, QuantityVector<dim> pQVector) :
        qVector(pQVector), cs(&pCS)
    {
    }
};

#endif
