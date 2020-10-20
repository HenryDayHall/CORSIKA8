/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>

namespace corsika {

  /*!
   * Common base class for Vector and Point. Currently it does basically nothing.
   */
  /*
   * FIXME Many potential issues:
   * 1. does this class really need to be templated ?
   * 2. copy constructor, assignment operator not implemented
   * 3. this member pointer is quite scary...
   */
  template <typename dim>
  class BaseVector {

  public:
    /*
     * FIXME Why to copy pQVector twice?
     */
    BaseVector(CoordinateSystem const& pCS, QuantityVector<dim> pQVector)
        : qVector(pQVector)
        , cs(&pCS) {}

    auto const& GetCoordinateSystem() const;

  protected:
    QuantityVector<dim> qVector;
    CoordinateSystem const* cs;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/BaseVector.inl>
