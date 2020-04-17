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
    inline auto QuantityVector<dim>::operator[](size_t index) const {
      return Quantity(phys::units::detail::magnitude_tag, eVector[index]);
    }

    template <typename dim>
    inline auto QuantityVector<dim>::GetX() const
    {
    	return (*this)[0];
    }

    template <typename dim>
    inline auto QuantityVector<dim>::GetY() const
    {
    	return (*this)[1];
    }

    template <typename dim>
    inline auto QuantityVector<dim>::GetZ() const
    {
    	return (*this)[2];
    }

    template <typename dim>
    inline auto QuantityVector<dim>::norm() const
    {
      return Quantity(phys::units::detail::magnitude_tag, eVector.norm());
    }

    template <typename dim>
    inline auto QuantityVector<dim>::squaredNorm() const
    {
      using QuantitySquared =
          decltype(std::declval<Quantity>() * std::declval<Quantity>());
      return QuantitySquared(phys::units::detail::magnitude_tag, eVector.squaredNorm());
    }

    template <typename dim>
    inline auto  QuantityVector<dim>::operator+(QuantityVector<dim> const& pQVec) const
    {
      return QuantityVector<dim>(eVector + pQVec.eVector);
    }

    template <typename dim>
    inline auto QuantityVector<dim>::operator-(QuantityVector<dim> const& pQVec) const
    {
      return QuantityVector<dim>(eVector - pQVec.eVector);
    }

    template <typename dim>
    template <typename ScalarDim>
    inline auto QuantityVector<dim>::operator*(phys::units::quantity<ScalarDim, double> const p) const
    {
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

    template <typename dim>
    template <typename ScalarDim>
    inline auto QuantityVector<dim>::operator/(phys::units::quantity<ScalarDim, double> const p) const
    {
      return (*this) * (1 / p);
    }

    template <typename dim>
    inline auto QuantityVector<dim>::operator*(double const p) const
    {
    	return QuantityVector<dim>(eVector * p);
    }

    template <typename dim>
    inline auto QuantityVector<dim>::operator/(double const p) const
    {
    	return QuantityVector<dim>(eVector / p);
    }

    template <typename dim>
    inline auto& QuantityVector<dim>::operator/=(double const p) {
      eVector /= p;
      return *this;
    }

    template <typename dim>
    inline auto& QuantityVector<dim>::operator*=(double const p) {
      eVector *= p;
      return *this;
    }

    template <typename dim>
    inline auto& QuantityVector<dim>::operator+=(QuantityVector<dim> const& pQVec) {
      eVector += pQVec.eVector;
      return *this;
    }

    template <typename dim>
    inline auto& QuantityVector<dim>::operator-=(QuantityVector<dim> const& pQVec) {
      eVector -= pQVec.eVector;
      return *this;
    }

    template <typename dim>
    inline auto& QuantityVector<dim>::operator-() const {
    	return QuantityVector<dim>(-eVector);
    }

    template <typename dim>
    inline auto QuantityVector<dim>::normalized() const { return QuantityVector<dim>(eVector.normalized()); }

    template <typename dim>
    inline auto QuantityVector<dim>::operator==(QuantityVector<dim> const& p) const { return eVector == p.eVector; }


  template <typename dim>
  inline auto& operator<<(std::ostream& os, corsika::QuantityVector<dim> qv) {
    using Quantity = phys::units::quantity<dim, double>;

    os << '(' << qv.eVector(0) << ' ' << qv.eVector(1) << ' ' << qv.eVector(2) << ") "
       << phys::units::to_unit_symbol<dim, double>(
              Quantity(phys::units::detail::magnitude_tag, 1));
    return os;
  }

} // namespace corsika

