#ifndef BASEVECTOR_H_
#define BASEVECTOR_H_

#include "QuantityVector.h"
#include "CoordinateSystem.h"

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
