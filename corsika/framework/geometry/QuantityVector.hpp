n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Eigen/Dense>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <iostream>
#include <utility>

namespace corsika {

  /*!
   * A QuantityVector is a three-component container based on Eigen::Vector3d
   * with a phys::units::si::dimension. Arithmethic operators are defined that
   * propagate the dimensions by dimensional analysis.
   */

  template <typename dim>
  class QuantityVector {
  public:
    using Quantity = phys::units::quantity<dim, double>; //< the phys::units::quantity
                                                         // corresponding to the dimension

  public:
    Eigen::Vector3d eVector; //!< the actual container where the raw numbers are stored

    typedef dim dimension; //!< should be a phys::units::dimension

    QuantityVector(Quantity a, Quantity b, Quantity c)
        : eVector{a.magnitude(), b.magnitude(), c.magnitude()} {}

    QuantityVector(double a, double b, double c)
        : eVector{a, b, c} {
      static_assert(
          std::is_same_v<dim, phys::units::dimensionless_d>,
          "initialization of dimensionful QuantityVector with pure numbers not allowed!");
    }

    QuantityVector(Eigen::Vector3d pBareVector)
        : eVector(pBareVector) {}

    auto operator[](size_t index) const;

    auto GetX() const;

    auto GetY() const;

    auto GetZ() const;

    auto norm() const;

    auto squaredNorm() const;

    auto operator+(QuantityVector<dim> const& pQVec) const;

    auto operator-(QuantityVector<dim> const& pQVec) const;

    template <typename ScalarDim>
    auto operator*(phys::units::quantity<ScalarDim, double> const p) const;

    template <typename ScalarDim>
    auto operator/(phys::units::quantity<ScalarDim, double> const p) const;

    auto operator*(double const p) const;

    auto operator/(double const p) const;

    auto& operator/=(double const p);

    auto& operator*=(double const p);

    auto& operator+=(QuantityVector<dim> const& pQVec);

    auto& operator-=(QuantityVector<dim> const& pQVec);

    auto& operator-() const;

    auto normalized() const;

    auto operator==(QuantityVector<dim> const& p) const;
  };

  /*
   * FIXME free function operators not implemented.
   */

  template <typename dim>
  auto& operator<<(std::ostream& os, corsika::QuantityVector<dim> qv);

} // namespace corsika

#include <corsika/detail/framework/geometry/QuantityVector.inl>
