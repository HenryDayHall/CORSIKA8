/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

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

    auto const& GetCoordinateSystem() const { return *cs; }
  };

} // namespace corsika::geometry

#endif
