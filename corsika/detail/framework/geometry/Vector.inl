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

#include <corsika/framework/geometry/BaseVector.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  template <typename dim>
  QuantityVector<dim> Vector<dim>::getComponents() const {
    return BaseVector<dim>::getQuantityVector();
  }

  template <typename dim>
  QuantityVector<dim> Vector<dim>::getComponents(CoordinateSystemPtr pCS) const {
    if (*pCS == *BaseVector<dim>::getCoordinateSystem()) { // FIXME
      return BaseVector<dim>::getQuantityVector();
    } else {
      return QuantityVector<dim>(
          getTransformation(BaseVector<dim>::getCoordinateSystem(), pCS).linear() *
          BaseVector<dim>::getQuantityVector().eigenVector_);
    }
  }

  template <typename dim>
  inline typename Vector<dim>::quantity_type Vector<dim>::getX(
      CoordinateSystemPtr pCS) const {
    if (*pCS == *BaseVector<dim>::getCoordinateSystem()) {
      return BaseVector<dim>::getQuantityVector()[0];
    } else {
      return QuantityVector<dim>(
          getTransformation(BaseVector<dim>::getCoordinateSystem(), pCS).linear() *
          BaseVector<dim>::getQuantityVector().eigenVector_)[0];
    }
  }

  template <typename dim>
  inline typename Vector<dim>::quantity_type Vector<dim>::getY(
      CoordinateSystemPtr pCS) const {
    if (*pCS == *BaseVector<dim>::getCoordinateSystem()) {
      return BaseVector<dim>::getQuantityVector()[1];
    } else {
      return QuantityVector<dim>(
          getTransformation(BaseVector<dim>::getCoordinateSystem(), pCS).linear() *
          BaseVector<dim>::getQuantityVector().eigenVector_)[1];
    }
  }

  template <typename dim>
  inline typename Vector<dim>::quantity_type Vector<dim>::getZ(
      CoordinateSystemPtr pCS) const {
    if (*pCS == *BaseVector<dim>::getCoordinateSystem()) {
      return BaseVector<dim>::getQuantityVector()[2];
    } else {
      return QuantityVector<dim>(
          getTransformation(BaseVector<dim>::getCoordinateSystem(), pCS).linear() *
          BaseVector<dim>::getQuantityVector().eigenVector_)[2];
    }
  }

  template <typename dim>
  void Vector<dim>::rebase(CoordinateSystemPtr pCS) {
    BaseVector<dim>::quantityVector() = getComponents(pCS);
    BaseVector<dim>::setCoordinateSystem(pCS);
  }

  template <typename dim>
  inline typename Vector<dim>::quantity_type Vector<dim>::getNorm() const {
    return BaseVector<dim>::getQuantityVector().getNorm();
  }

  template <typename dim>
  auto Vector<dim>::getSquaredNorm() const {
    return BaseVector<dim>::getQuantityVector().getSquaredNorm();
  }

  template <typename dim>
  template <typename dim2>
  auto Vector<dim>::getParallelProjectionOnto(Vector<dim2> const& pVec,
                                              CoordinateSystemPtr pCS) const {
    auto const ourCompVec = getComponents(pCS);
    auto const otherCompVec = pVec.getComponents(pCS);
    auto const& a = ourCompVec.eigenVector_;
    auto const& b = otherCompVec.eigenVector_;

    return Vector<dim>(pCS, QuantityVector<dim>(b * ((a.dot(b)) / b.squaredNorm())));
  }

  template <typename dim>
  template <typename dim2>
  auto Vector<dim>::getParallelProjectionOnto(Vector<dim2> const& pVec) const {
    return getParallelProjectionOnto<dim2>(pVec, BaseVector<dim>::getCoordinateSystem());
  }

  template <typename dim>
  Vector<dim> Vector<dim>::operator+(Vector<dim> const& pVec) const {
    auto const components = getComponents(BaseVector<dim>::getCoordinateSystem()) +
                            pVec.getComponents(BaseVector<dim>::getCoordinateSystem());
    return Vector<dim>(BaseVector<dim>::getCoordinateSystem(), components);
  }

  template <typename dim>
  Vector<dim> Vector<dim>::operator-(Vector<dim> const& pVec) const {
    auto const components =
        getComponents() - pVec.getComponents(BaseVector<dim>::getCoordinateSystem());
    return Vector<dim>(BaseVector<dim>::getCoordinateSystem(), components);
  }

  template <typename dim>
  auto& Vector<dim>::operator*=(double const p) {
    BaseVector<dim>::quantityVector() *= p;
    return *this;
  }

  template <typename dim>
  template <typename ScalarDim>
  auto Vector<dim>::operator*(phys::units::quantity<ScalarDim, double> const p) const {
    using ProdDim = phys::units::detail::product_d<dim, ScalarDim>;

    return Vector<ProdDim>(BaseVector<dim>::getCoordinateSystem(),
                           BaseVector<dim>::getQuantityVector() * p);
  }

  template <typename dim>
  template <typename ScalarDim>
  auto Vector<dim>::operator/(phys::units::quantity<ScalarDim, double> const p) const {
    return (*this) * (1 / p);
  }

  template <typename dim>
  auto Vector<dim>::operator*(double const p) const {
    return Vector<dim>(BaseVector<dim>::getCoordinateSystem(),
                       BaseVector<dim>::getQuantityVector() * p);
  }

  template <typename dim>
  auto Vector<dim>::operator/(double const p) const {
    return Vector<dim>(BaseVector<dim>::getCoordinateSystem(),
                       BaseVector<dim>::quantityVector() / p);
  }

  template <typename dim>
  auto& Vector<dim>::operator+=(Vector<dim> const& pVec) {
    BaseVector<dim>::quantityVector() +=
        pVec.getComponents(BaseVector<dim>::getCoordinateSystem());
    return *this;
  }

  template <typename dim>
  auto& Vector<dim>::operator-=(Vector<dim> const& pVec) {
    BaseVector<dim>::quantityVector() -=
        pVec.getComponents(BaseVector<dim>::getCoordinateSystem());
    return *this;
  }

  template <typename dim>
  auto& Vector<dim>::operator-() const {
    return Vector<dim>(BaseVector<dim>::getCoordinateSystem(),
                       -BaseVector<dim>::quantityVector());
  }

  template <typename dim>
  auto Vector<dim>::normalized() const {
    return (*this) * (1 / getNorm());
  }

  template <typename dim>
  template <typename dim2>
  auto Vector<dim>::cross(Vector<dim2> pV) const {
    auto const c1 = getComponents().eigenVector_;
    auto const c2 = pV.getComponents(BaseVector<dim>::getCoordinateSystem()).eigenVector_;
    auto const bareResult = c1.cross(c2);

    using ProdDim = phys::units::detail::product_d<dim, dim2>;
    return Vector<ProdDim>(BaseVector<dim>::getCoordinateSystem(), bareResult);
  }

  template <typename dim>
  template <typename dim2>
  auto Vector<dim>::dot(Vector<dim2> pV) const {
    auto const c1 = getComponents().eigenVector_;
    auto const c2 = pV.getComponents(BaseVector<dim>::getCoordinateSystem()).eigenVector_;
    auto const bareResult = c1.dot(c2);

    using ProdDim = phys::units::detail::product_d<dim, dim2>;

    return phys::units::quantity<ProdDim, double>(phys::units::detail::magnitude_tag,
                                                  bareResult);
  }

} // namespace corsika
