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

  auto Point::getCoordinates() const { return BaseVector<length_d>::getQuantityVector(); }

  inline LengthType Point::getX(CoordinateSystemPtr cs) const {
    if (*cs == *BaseVector<length_d>::getCoordinateSystem()) {
      return BaseVector<length_d>::getQuantityVector().getX();
    } else {
      return QuantityVector<length_d>(
                 getTransformation(BaseVector<length_d>::getCoordinateSystem(), cs) *
                 BaseVector<length_d>::getQuantityVector().eigenVector_)
          .getX();
    }
  }

  inline LengthType Point::getY(CoordinateSystemPtr cs) const {
    if (*cs == *BaseVector<length_d>::getCoordinateSystem()) {
      return BaseVector<length_d>::getQuantityVector().getY();
    } else {
      return QuantityVector<length_d>(
                 getTransformation(BaseVector<length_d>::getCoordinateSystem(), cs) *
                 BaseVector<length_d>::getQuantityVector().eigenVector_)
          .getY();
    }
  }

  inline LengthType Point::getZ(CoordinateSystemPtr cs) const {
    if (*cs == *BaseVector<length_d>::getCoordinateSystem()) {
      return BaseVector<length_d>::getQuantityVector().getZ();
    } else {
      return QuantityVector<length_d>(
                 getTransformation(BaseVector<length_d>::getCoordinateSystem(), cs) *
                 BaseVector<length_d>::getQuantityVector().eigenVector_)
          .getZ();
    }
  }

  /// this always returns a QuantityVector as triple
  auto Point::getCoordinates(CoordinateSystemPtr pCS) const {
    if (*pCS == *BaseVector<length_d>::getCoordinateSystem()) {
      return BaseVector<length_d>::getQuantityVector();
    } else {
      return QuantityVector<length_d>(
          getTransformation(BaseVector<length_d>::getCoordinateSystem(), pCS) *
          BaseVector<length_d>::getQuantityVector().eigenVector_);
    }
  }

  /*!
   * transforms the Point into another CoordinateSystem by changing its
   * coordinates interally
   */
  void Point::rebase(CoordinateSystemPtr pCS) {
    BaseVector<length_d>::quantityVector() = getCoordinates(pCS);
    BaseVector<length_d>::setCoordinateSystem(pCS);
  }

  Point Point::operator+(Vector<length_d> const& pVec) const {
    return Point(BaseVector<length_d>::getCoordinateSystem(),
                 getCoordinates() +
                     pVec.getComponents(BaseVector<length_d>::getCoordinateSystem()));
  }

  /*!
   * returns the distance Vector between two points
   */
  Vector<length_d> Point::operator-(Point const& pB) const {
    CoordinateSystemPtr cs = BaseVector<length_d>::getCoordinateSystem();
    return Vector<length_d>(cs, getCoordinates() - pB.getCoordinates(cs));
  }

} // namespace corsika
