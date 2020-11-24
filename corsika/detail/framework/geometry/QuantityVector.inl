/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Eigen/Dense>

#include <iostream>
#include <utility>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  template <typename dim>
  inline typename QuantityVector<dim>::quantity_type QuantityVector<dim>::operator[](
      size_t index) const {
    return quantity_type(phys::units::detail::magnitude_tag, eigenVector_[index]);
  }

  template <typename dim>
  inline typename QuantityVector<dim>::quantity_type QuantityVector<dim>::getX() const {
    return (*this)[0];
  }

  template <typename dim>
  inline typename QuantityVector<dim>::quantity_type QuantityVector<dim>::getY() const {
    return (*this)[1];
  }

  template <typename dim>
  inline typename QuantityVector<dim>::quantity_type QuantityVector<dim>::getZ() const {
    return (*this)[2];
  }

  template <typename dim>
  inline typename QuantityVector<dim>::quantity_type QuantityVector<dim>::getNorm()
      const {
    return quantity_type(phys::units::detail::magnitude_tag, eigenVector_.norm());
  }

  template <typename dim>
  inline auto QuantityVector<dim>::getSquaredNorm() const {
    using QuantitySquared =
        decltype(std::declval<quantity_type>() * std::declval<quantity_type>());
    return QuantitySquared(phys::units::detail::magnitude_tag,
                           eigenVector_.squaredNorm());
  }

  template <typename dim>
  inline QuantityVector<dim> QuantityVector<dim>::operator+(
      QuantityVector<dim> const& pQVec) const {
    return QuantityVector<dim>(eigenVector_ + pQVec.eigenVector_);
  }

  template <typename dim>
  inline QuantityVector<dim> QuantityVector<dim>::operator-(
      QuantityVector<dim> const& pQVec) const {
    return QuantityVector<dim>(eigenVector_ - pQVec.eigenVector_);
  }

  template <typename dim>
  template <typename ScalarDim>
  inline auto QuantityVector<dim>::operator*(
      phys::units::quantity<ScalarDim, double> const p) const {
    using ResQuantity = phys::units::detail::Product<ScalarDim, dim, double, double>;

    if constexpr (std::is_same<ResQuantity, double>::value) // result dimensionless, not
                                                            // a "quantity_type" anymore
    {
      return QuantityVector<phys::units::dimensionless_d>(eigenVector_ * p.magnitude());
    } else {
      return QuantityVector<typename ResQuantity::dimension_type>(eigenVector_ *
                                                                  p.magnitude());
    }
  }

  template <typename dim>
  template <typename ScalarDim>
  inline auto QuantityVector<dim>::operator/(
      phys::units::quantity<ScalarDim, double> const p) const {
    return (*this) * (1 / p);
  }

  template <typename dim>
  inline auto QuantityVector<dim>::operator*(double const p) const {
    return QuantityVector<dim>(eigenVector_ * p);
  }

  template <typename dim>
  inline auto QuantityVector<dim>::operator/(double const p) const {
    return QuantityVector<dim>(eigenVector_ / p);
  }

  template <typename dim>
  inline auto& QuantityVector<dim>::operator/=(double const p) {
    eigenVector_ /= p;
    return *this;
  }

  template <typename dim>
  inline auto& QuantityVector<dim>::operator*=(double const p) {
    eigenVector_ *= p;
    return *this;
  }

  template <typename dim>
  inline auto& QuantityVector<dim>::operator+=(QuantityVector<dim> const& pQVec) {
    eigenVector_ += pQVec.eigenVector_;
    return *this;
  }

  template <typename dim>
  inline auto& QuantityVector<dim>::operator-=(QuantityVector<dim> const& pQVec) {
    eigenVector_ -= pQVec.eigenVector_;
    return *this;
  }

  template <typename dim>
  inline auto& QuantityVector<dim>::operator-() const {
    return QuantityVector<dim>(-eigenVector_);
  }

  template <typename dim>
  inline auto QuantityVector<dim>::normalized() const {
    return QuantityVector<dim>(eigenVector_.normalized());
  }

  template <typename dim>
  inline auto QuantityVector<dim>::operator==(QuantityVector<dim> const& p) const {
    return eigenVector_ == p.eigenVector_;
  }

  template <typename dim>
  inline std::ostream& operator<<(std::ostream& os, corsika::QuantityVector<dim> qv) {
    using quantity_type = phys::units::quantity<dim, double>;

    os << '(' << qv.eigenVector_(0) << ' ' << qv.eigenVector_(1) << ' '
       << qv.eigenVector_(2) << ") "
       << phys::units::to_unit_symbol<dim, double>(
              quantity_type(phys::units::detail::magnitude_tag, 1));
    return os;
  }

} // namespace corsika
