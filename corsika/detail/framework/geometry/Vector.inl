/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/BaseVector.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  template <typename TDimension>
  inline QuantityVector<TDimension> const& Vector<TDimension>::getComponents() const {
    return BaseVector<TDimension>::getQuantityVector();
  }

  template <typename TDimension>
  inline QuantityVector<TDimension>& Vector<TDimension>::getComponents() {
    return BaseVector<TDimension>::getQuantityVector();
  }

  template <typename TDimension>
  inline QuantityVector<TDimension> Vector<TDimension>::getComponents(
      CoordinateSystemPtr const& pCS) const {
    if (*pCS == *BaseVector<TDimension>::getCoordinateSystem()) {
      return BaseVector<TDimension>::getQuantityVector();
    } else {
      return QuantityVector<TDimension>(
          get_transformation(BaseVector<TDimension>::getCoordinateSystem(), pCS)
              .linear() *
          BaseVector<TDimension>::getQuantityVector().eigenVector_);
    }
  }

  template <typename TDimension>
  inline QuantityVector<TDimension>& Vector<TDimension>::getComponents(
      CoordinateSystemPtr const& pCS) {
    if (*pCS != *BaseVector<TDimension>::getCoordinateSystem()) { rebase(pCS); }
    return BaseVector<TDimension>::getQuantityVector();
  }

  template <typename TDimension>
  inline typename Vector<TDimension>::quantity_type Vector<TDimension>::getX(
      CoordinateSystemPtr const& pCS) const {
    if (*pCS == *BaseVector<TDimension>::getCoordinateSystem()) {
      return BaseVector<TDimension>::getQuantityVector()[0];
    } else {
      return QuantityVector<TDimension>(
          get_transformation(BaseVector<TDimension>::getCoordinateSystem(), pCS)
              .linear() *
          BaseVector<TDimension>::getQuantityVector().eigenVector_)[0];
    }
  }

  template <typename TDimension>
  inline typename Vector<TDimension>::quantity_type Vector<TDimension>::getY(
      CoordinateSystemPtr const& pCS) const {
    if (*pCS == *BaseVector<TDimension>::getCoordinateSystem()) {
      return BaseVector<TDimension>::getQuantityVector()[1];
    } else {
      return QuantityVector<TDimension>(
          get_transformation(BaseVector<TDimension>::getCoordinateSystem(), pCS)
              .linear() *
          BaseVector<TDimension>::getQuantityVector().eigenVector_)[1];
    }
  }

  template <typename TDimension>
  inline typename Vector<TDimension>::quantity_type Vector<TDimension>::getZ(
      CoordinateSystemPtr const& pCS) const {
    if (*pCS == *BaseVector<TDimension>::getCoordinateSystem()) {
      return BaseVector<TDimension>::getQuantityVector()[2];
    } else {
      return QuantityVector<TDimension>(
          get_transformation(BaseVector<TDimension>::getCoordinateSystem(), pCS)
              .linear() *
          BaseVector<TDimension>::getQuantityVector().eigenVector_)[2];
    }
  }

  template <typename TDimension>
  void Vector<TDimension>::rebase(CoordinateSystemPtr const& pCS) {
    BaseVector<TDimension>::setQuantityVector(QuantityVector<TDimension>(
        get_transformation(BaseVector<TDimension>::getCoordinateSystem(), pCS).linear() *
        BaseVector<TDimension>::getQuantityVector().eigenVector_));
    BaseVector<TDimension>::setCoordinateSystem(pCS);
  }

  template <typename TDimension>
  inline typename Vector<TDimension>::quantity_type Vector<TDimension>::getNorm() const {
    return BaseVector<TDimension>::getQuantityVector().getNorm();
  }

  template <typename TDimension>
  inline typename Vector<TDimension>::quantity_square_type
  Vector<TDimension>::getSquaredNorm() const {
    return BaseVector<TDimension>::getQuantityVector().getSquaredNorm();
  }

  template <typename TDimension>
  template <typename TDimension2>
  auto Vector<TDimension>::getParallelProjectionOnto(
      Vector<TDimension2> const& pVec, CoordinateSystemPtr const& pCS) const {
    auto const ourCompVec = getComponents(pCS);
    auto const otherCompVec = pVec.getComponents(pCS);
    auto const& a = ourCompVec.eigenVector_;
    auto const& b = otherCompVec.eigenVector_;

    return Vector<TDimension>(
        pCS, QuantityVector<TDimension>(b * ((a.dot(b)) / b.squaredNorm())));
  }

  template <typename TDimension>
  template <typename TDimension2>
  auto Vector<TDimension>::getParallelProjectionOnto(
      Vector<TDimension2> const& pVec) const {
    return getParallelProjectionOnto<TDimension2>(
        pVec, BaseVector<TDimension>::getCoordinateSystem());
  }

  template <typename TDimension>
  Vector<TDimension> Vector<TDimension>::operator+(Vector<TDimension> const& pVec) const {
    CoordinateSystemPtr const& cs = BaseVector<TDimension>::getCoordinateSystem();
    auto const components = getComponents(cs) + pVec.getComponents(cs);
    return Vector<TDimension>(BaseVector<TDimension>::getCoordinateSystem(), components);
  }

  template <typename TDimension>
  Vector<TDimension> Vector<TDimension>::operator-(Vector<TDimension> const& pVec) const {
    CoordinateSystemPtr const& cs = BaseVector<TDimension>::getCoordinateSystem();
    return Vector<TDimension>(cs, getComponents() - pVec.getComponents(cs));
  }

  template <typename TDimension>
  auto& Vector<TDimension>::operator*=(double const p) {
    BaseVector<TDimension>::getQuantityVector() *= p;
    return *this;
  }

  template <typename TDimension>
  template <typename TScalarDim>
  auto Vector<TDimension>::operator*(
      phys::units::quantity<TScalarDim, double> const p) const {
    using ProdDim = phys::units::detail::product_d<TDimension, TScalarDim>;

    return Vector<ProdDim>(BaseVector<TDimension>::getCoordinateSystem(),
                           BaseVector<TDimension>::getQuantityVector() * p);
  }

  template <typename TDimension>
  template <typename TScalarDim>
  auto Vector<TDimension>::operator/(
      phys::units::quantity<TScalarDim, double> const p) const {
    return (*this) * (1 / p);
  }

  template <typename TDimension>
  auto Vector<TDimension>::operator*(double const p) const {
    return Vector<TDimension>(BaseVector<TDimension>::getCoordinateSystem(),
                              BaseVector<TDimension>::getQuantityVector() * p);
  }

  template <typename TDimension>
  auto Vector<TDimension>::operator/(double const p) const {
    return Vector<TDimension>(BaseVector<TDimension>::getCoordinateSystem(),
                              BaseVector<TDimension>::quantityVector() / p);
  }

  template <typename TDimension>
  auto& Vector<TDimension>::operator+=(Vector<TDimension> const& pVec) {
    BaseVector<TDimension>::getQuantityVector() +=
        pVec.getComponents(BaseVector<TDimension>::getCoordinateSystem());
    return *this;
  }

  template <typename TDimension>
  auto& Vector<TDimension>::operator-=(Vector<TDimension> const& pVec) {
    BaseVector<TDimension>::getQuantityVector() -=
        pVec.getComponents(BaseVector<TDimension>::getCoordinateSystem());
    return *this;
  }

  template <typename TDimension>
  auto& Vector<TDimension>::operator-() const {
    return Vector<TDimension>(BaseVector<TDimension>::getCoordinateSystem(),
                              -BaseVector<TDimension>::getQuantityVector());
  }

  template <typename TDimension>
  auto Vector<TDimension>::normalized() const {
    return (*this) * (1 / getNorm());
  }

  template <typename TDimension>
  template <typename TDimension2>
  auto Vector<TDimension>::cross(Vector<TDimension2> const& pV) const {
    auto const c1 = getComponents().eigenVector_;
    auto const c2 =
        pV.getComponents(BaseVector<TDimension>::getCoordinateSystem()).eigenVector_;
    auto const bareResult = c1.cross(c2);

    using ProdDim = phys::units::detail::product_d<TDimension, TDimension2>;
    return Vector<ProdDim>(BaseVector<TDimension>::getCoordinateSystem(), bareResult);
  }

  template <typename TDimension>
  template <typename TDimension2>
  auto Vector<TDimension>::dot(Vector<TDimension2> const& pV) const {
    auto const c1 = getComponents().eigenVector_;
    auto const c2 =
        pV.getComponents(BaseVector<TDimension>::getCoordinateSystem()).eigenVector_;
    auto const bareResult = c1.dot(c2);

    using ProdDim = phys::units::detail::product_d<TDimension, TDimension2>;

    return phys::units::quantity<ProdDim, double>(phys::units::detail::magnitude_tag,
                                                  bareResult);
  }

} // namespace corsika
