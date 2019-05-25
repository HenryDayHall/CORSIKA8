/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_POINT_H_
#define _include_POINT_H_

#include <corsika/geometry/BaseVector.h>
#include <corsika/geometry/QuantityVector.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  using corsika::units::si::length_d;
  using corsika::units::si::LengthType;

  /*!
   * A Point represents a point in position space. It is defined by its
   * coordinates with respect to some CoordinateSystem.
   */
  class Point : public BaseVector<length_d> {
  public:
    Point(CoordinateSystem const& pCS, QuantityVector<length_d> pQVector)
        : BaseVector<length_d>(pCS, pQVector) {}

    Point(CoordinateSystem const& cs, LengthType x, LengthType y, LengthType z)
        : BaseVector<length_d>(cs, {x, y, z}) {}

    // TODO: this should be private or protected, we don NOT want to expose numbers
    // without reference to outside:
    auto GetCoordinates() const { return BaseVector<length_d>::qVector; }
    auto GetX() const { return BaseVector<length_d>::qVector.GetX(); }
    auto GetY() const { return BaseVector<length_d>::qVector.GetY(); }
    auto GetZ() const { return BaseVector<length_d>::qVector.GetZ(); }

    /// this always returns a QuantityVector as triple
    auto GetCoordinates(CoordinateSystem const& pCS) const {
      if (&pCS == BaseVector<length_d>::cs) {
        return BaseVector<length_d>::qVector;
      } else {
        return QuantityVector<length_d>(
            CoordinateSystem::GetTransformation(*BaseVector<length_d>::cs, pCS) *
            BaseVector<length_d>::qVector.eVector);
      }
    }

    /*!
     * transforms the Point into another CoordinateSystem by changing its
     * coordinates interally
     */
    void rebase(CoordinateSystem const& pCS) {
      BaseVector<length_d>::qVector = GetCoordinates(pCS);
      BaseVector<length_d>::cs = &pCS;
    }

    Point operator+(Vector<length_d> const& pVec) const {
      return Point(*BaseVector<length_d>::cs,
                   GetCoordinates() + pVec.GetComponents(*BaseVector<length_d>::cs));
    }

    /*!
     * returns the distance Vector between two points
     */
    Vector<length_d> operator-(Point const& pB) const {
      auto& cs = *BaseVector<length_d>::cs;
      return Vector<length_d>(cs, GetCoordinates() - pB.GetCoordinates(cs));
    }
  };

} // namespace corsika::geometry

#endif
