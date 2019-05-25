/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_QUANTITYVECTOR_H_
#define _include_QUANTITYVECTOR_H_

#include <corsika/units/PhysicalUnits.h>

#include <Eigen/Dense>

#include <iostream>
#include <utility>

namespace corsika::geometry {

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

    auto operator[](size_t index) const {
      return Quantity(phys::units::detail::magnitude_tag, eVector[index]);
    }

    auto GetX() const { return Quantity(phys::units::detail::magnitude_tag, eVector[0]); }

    auto GetY() const { return Quantity(phys::units::detail::magnitude_tag, eVector[1]); }

    auto GetZ() const { return Quantity(phys::units::detail::magnitude_tag, eVector[2]); }

    auto norm() const {
      return Quantity(phys::units::detail::magnitude_tag, eVector.norm());
    }

    auto squaredNorm() const {
      using QuantitySquared =
          decltype(std::declval<Quantity>() * std::declval<Quantity>());
      return QuantitySquared(phys::units::detail::magnitude_tag, eVector.squaredNorm());
    }

    auto operator+(QuantityVector<dim> const& pQVec) const {
      return QuantityVector<dim>(eVector + pQVec.eVector);
    }

    auto operator-(QuantityVector<dim> const& pQVec) const {
      return QuantityVector<dim>(eVector - pQVec.eVector);
    }

    template <typename ScalarDim>
    auto operator*(phys::units::quantity<ScalarDim, double> const p) const {
      using ResQuantity = phys::units::detail::Product<ScalarDim, dim, double, double>;

      if constexpr (std::is_same<ResQuantity, double>::value) // result dimensionless, not
                                                              // a "Quantity" anymore
      {
        return QuantityVector<phys::units::dimensionless_d>(eVector * p.magnitude());
      } else {
        return QuantityVector<typename ResQuantity::dimension_type>(eVector *
                                                                    p.magnitude());
      }
    }

    template <typename ScalarDim>
    auto operator/(phys::units::quantity<ScalarDim, double> const p) const {
      return (*this) * (1 / p);
    }

    auto operator*(double const p) const { return QuantityVector<dim>(eVector * p); }

    auto operator/(double const p) const { return QuantityVector<dim>(eVector / p); }

    auto& operator/=(double const p) {
      eVector /= p;
      return *this;
    }

    auto& operator*=(double const p) {
      eVector *= p;
      return *this;
    }

    auto& operator+=(QuantityVector<dim> const& pQVec) {
      eVector += pQVec.eVector;
      return *this;
    }

    auto& operator-=(QuantityVector<dim> const& pQVec) {
      eVector -= pQVec.eVector;
      return *this;
    }

    auto& operator-() const { return QuantityVector<dim>(-eVector); }

    auto normalized() const { return QuantityVector<dim>(eVector.normalized()); }

    auto operator==(QuantityVector<dim> const& p) const { return eVector == p.eVector; }
  };

  template <typename dim>
  auto& operator<<(std::ostream& os, corsika::geometry::QuantityVector<dim> qv) {
    using Quantity = phys::units::quantity<dim, double>;

    os << '(' << qv.eVector(0) << ' ' << qv.eVector(1) << ' ' << qv.eVector(2) << ") "
       << phys::units::to_unit_symbol<dim, double>(
              Quantity(phys::units::detail::magnitude_tag, 1));
    return os;
  }

} // namespace corsika::geometry

#endif
