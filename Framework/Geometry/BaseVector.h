#ifndef _include_BASEVECTOR_H_
#define _include_BASEVECTOR_H_

#include <fwk/QuantityVector.h>
#include <fwk/CoordinateSystem.h>

namespace fwk {

/*!
 * Common base class for Vector and Point. Currently it does basically nothing.
 */

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

} // end namesapce
 
#endif
