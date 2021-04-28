/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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

  inline QuantityVector<length_d> const& Point::getCoordinates() const {
    return BaseVector<length_d>::getQuantityVector();
  }

  inline QuantityVector<length_d>& Point::getCoordinates() {
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
  inline QuantityVector<length_d> Point::getCoordinates(
      CoordinateSystemPtr const& pCS) const {
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
  inline QuantityVector<length_d>& Point::getCoordinates(CoordinateSystemPtr const& pCS) {
    if (*pCS != *BaseVector<length_d>::getCoordinateSystem()) { rebase(pCS); }
    return BaseVector<length_d>::getQuantityVector();
  }

  inline void Point::rebase(CoordinateSystemPtr const& pCS) {
    BaseVector<length_d>::setQuantityVector(QuantityVector<length_d>(
        get_transformation(*BaseVector<length_d>::getCoordinateSystem().get(),
                           *pCS.get()) *
        BaseVector<length_d>::getQuantityVector().eigenVector_));
    BaseVector<length_d>::setCoordinateSystem(pCS);
  }

  inline Point Point::operator+(Vector<length_d> const& pVec) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    return Point(cs, getCoordinates() + pVec.getComponents(cs));
  }

  inline Vector<length_d> Point::operator-(Point const& pB) const {
    CoordinateSystemPtr const& cs = BaseVector<length_d>::getCoordinateSystem();
    return Vector<length_d>(cs, getCoordinates() - pB.getCoordinates(cs));
  }

  inline std::ostream& operator<<(std::ostream& os, corsika::Point const& p) {
    auto const& qv = p.getCoordinates();
    os << qv << " (ref:" << fmt::ptr(p.getCoordinateSystem()) << ")";
    return os;
  }

  inline LengthType distance(Point const &p1, Point const &p2) {
    return (p1 - p2).getNorm();
  }

} // namespace corsika