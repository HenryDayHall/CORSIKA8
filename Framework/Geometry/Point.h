
/**
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
  class Point : public BaseVector<phys::units::length_d> {
  public:
    Point(CoordinateSystem const& pCS, QuantityVector<phys::units::length_d> pQVector)
        : BaseVector<phys::units::length_d>(pCS, pQVector) {}

    Point(CoordinateSystem const& cs, LengthType x, LengthType y, LengthType z)
        : BaseVector<phys::units::length_d>(cs, {x, y, z}) {}

    // TODO: this should be private or protected, we don NOT want to expose numbers without reference to outside:
    auto GetCoordinates() const { return BaseVector<phys::units::length_d>::qVector; }

    /// this always returns a QuantityVector as triple
    auto GetCoordinates(CoordinateSystem const& pCS) const {
      if (&pCS == BaseVector<phys::units::length_d>::cs) {
        return BaseVector<phys::units::length_d>::qVector;
      } else {
        return QuantityVector<phys::units::length_d>(
            CoordinateSystem::GetTransformation(*BaseVector<phys::units::length_d>::cs,
                                                pCS) *
            BaseVector<phys::units::length_d>::qVector.eVector);
      }
    }

    /*!
     * transforms the Point into another CoordinateSystem by changing its
     * coordinates interally
     */
    void rebase(CoordinateSystem const& pCS) {
      BaseVector<phys::units::length_d>::qVector = GetCoordinates(pCS);
      BaseVector<phys::units::length_d>::cs = &pCS;
    }

    Point operator+(Vector<phys::units::length_d> const& pVec) const {
      return Point(
          *BaseVector<phys::units::length_d>::cs,
          GetCoordinates() + pVec.GetComponents(*BaseVector<phys::units::length_d>::cs));
    }

    /*!
     * returns the distance Vector between two points
     */
    Vector<phys::units::length_d> operator-(Point const& pB) const {
      auto& cs = *BaseVector<phys::units::length_d>::cs;
      return Vector<phys::units::length_d>(cs, GetCoordinates() - pB.GetCoordinates(cs));
    }
  };

} // namespace corsika::geometry

#endif
