#ifndef _include_BASEVECTOR_H_
#define _include_BASEVECTOR_H_

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/QuantityVector.h>

namespace corsika::geometry {

  /*!
   * Common base class for Vector and Point. Currently it does basically nothing.
   */

  template <typename dim>
  class BaseVector {
  protected:
    QuantityVector<dim> qVector;
    CoordinateSystem const* cs;

  public:
    BaseVector(CoordinateSystem const& pCS, QuantityVector<dim> pQVector)
        : qVector(pQVector)
        , cs(&pCS) {}
  };

} // namespace corsika::geometry

#endif
