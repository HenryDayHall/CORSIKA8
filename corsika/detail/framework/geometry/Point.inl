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
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  QuantityVector<length_d> const& Point::getCoordinates() const {
    return BaseVector<length_d>::getQuantityVector();
  }

  QuantityVector<length_d>& Point::getCoordinates() {
    return BaseVector<length_d>::getQuantityVector();
  }

  inline LengthType Point::getX(CoordinateSystemPtr const& pCS) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    if (*pCS == *cs) {
      return BaseVector<length_d>::getQuantityVector().getX();
    } else {
      return QuantityVector<length_d>(
                 get_transformation(*cs.get(), *pCS.get()) *
                 BaseVector<length_d>::getQuantityVector().eigenVector_)
          .getX();
    }
  }

  inline LengthType Point::getY(CoordinateSystemPtr const& pCS) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    if (*pCS == *cs) {
      return BaseVector<length_d>::getQuantityVector().getY();
    } else {
      return QuantityVector<length_d>(
                 get_transformation(*cs.get(), *pCS.get()) *
                 BaseVector<length_d>::getQuantityVector().eigenVector_)
          .getY();
    }
  }

  inline LengthType Point::getZ(CoordinateSystemPtr const& pCS) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    if (*pCS == *cs) {
      return BaseVector<length_d>::getQuantityVector().getZ();
    } else {
      return QuantityVector<length_d>(
                 get_transformation(*cs.get(), *pCS.get()) *
                 BaseVector<length_d>::getQuantityVector().eigenVector_)
          .getZ();
    }
  }

  /// this always returns a QuantityVector as triple
  QuantityVector<length_d> Point::getCoordinates(CoordinateSystemPtr const& pCS) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    if (*pCS == *cs) {
      return BaseVector<length_d>::getQuantityVector();
    } else {
      return QuantityVector<length_d>(
          get_transformation(*cs.get(), *pCS.get()) *
          BaseVector<length_d>::getQuantityVector().eigenVector_);
    }
  }

  /// this always returns a QuantityVector as triple
  QuantityVector<length_d>& Point::getCoordinates(CoordinateSystemPtr const& pCS) {
    if (*pCS != *BaseVector<length_d>::getCoordinateSystem()) { rebase(pCS); }
    return BaseVector<length_d>::getQuantityVector();
  }

  void Point::rebase(CoordinateSystemPtr const& pCS) {
    BaseVector<length_d>::setQuantityVector(QuantityVector<length_d>(
        get_transformation(*BaseVector<length_d>::getCoordinateSystem().get(),
                           *pCS.get()) *
        BaseVector<length_d>::getQuantityVector().eigenVector_));
    BaseVector<length_d>::setCoordinateSystem(pCS);
  }

  Point Point::operator+(Vector<length_d> const& pVec) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    return Point(cs, getCoordinates() + pVec.getComponents(cs));
  }

  Vector<length_d> Point::operator-(Point const& pB) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    return Vector<length_d>(cs, getCoordinates() - pB.getCoordinates(cs));
  }

} // namespace corsika
