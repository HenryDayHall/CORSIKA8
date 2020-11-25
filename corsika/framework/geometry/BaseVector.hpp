n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>

#include <memory>

namespace corsika {

  /*!
   * Common base class for Vector and Point.
   *
   * This holds a QuantityVector and a CoordinateSystem. The
   * BaseVector manages resources for many geometry objects.
   *
   */
  template <typename TDimension>
  class BaseVector {

  public:
    BaseVector(CoordinateSystemPtr const& pCS, QuantityVector<TDimension> const& pQVector)
        : quantityVector_(pQVector)
        , cs_(pCS) {}

    BaseVector() = delete; // we only want to creat initialized
			   // objects
    BaseVector(BaseVector const&) = default;
    BaseVector(BaseVector&& a) = default;
    BaseVector& operator=(BaseVector const&) = default;
    ~BaseVector() = default;

    CoordinateSystemPtr getCoordinateSystem() const;
    void setCoordinateSystem(CoordinateSystemPtr const&  cs) { cs_ = cs; }

  protected:
    QuantityVector<TDimension> const& getQuantityVector() const;
    QuantityVector<TDimension>& getQuantityVector();
    void setQuantityVector(QuantityVector<TDimension> const& v) { quantityVector_=v; }

  private:
    QuantityVector<TDimension> quantityVector_;
    CoordinateSystemPtr cs_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/BaseVector.inl>
