/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/BaseVector.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  /*!
   * A Point represents a point in position space. It is defined by its
   * coordinates with respect to some CoordinateSystem.
   */
  class Point : public BaseVector<length_d> {

  public:
    Point(CoordinateSystemPtr pCS, QuantityVector<length_d> const& pQVector)
        : BaseVector<length_d>(pCS, pQVector) {}

    Point(CoordinateSystemPtr cs, LengthType x, LengthType y, LengthType z)
        : BaseVector<length_d>(cs, {x, y, z}) {}

    /** \todo TODO: this should be private or protected, we don NOT want to expose numbers
     * without reference to outside:
     */
    inline auto getCoordinates() const;

    /// this always returns a QuantityVector as triple
    inline auto getCoordinates(CoordinateSystemPtr pCS) const;

    inline LengthType getX(CoordinateSystemPtr pCS) const;
    inline LengthType getY(CoordinateSystemPtr pCS) const;
    inline LengthType getZ(CoordinateSystemPtr pCS) const;

    /*!
     * transforms the Point into another CoordinateSystem by changing its
     * coordinates interally
     */
    inline void rebase(CoordinateSystemPtr pCS);

    inline Point operator+(Vector<length_d> const& pVec) const;

    /*!
     * returns the distance Vector between two points
     */
    inline Vector<length_d> operator-(Point const& pB) const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Point.inl>
