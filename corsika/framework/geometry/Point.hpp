/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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

  // FIXME: remove aliasing here
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
    inline auto GetCoordinates() const;

    inline auto GetX() const;

    inline auto GetY() const;

    inline auto GetZ() const;

    /// this always returns a QuantityVector as triple
    inline auto GetCoordinates(CoordinateSystem const& pCS) const;

    /*!
     * transforms the Point into another CoordinateSystem by changing its
     * coordinates interally
     */
    inline void rebase(CoordinateSystem const& pCS);

    inline Point operator+(Vector<length_d> const& pVec) const;

    /*!
     * returns the distance Vector between two points
     */
    inline Vector<length_d> operator-(Point const& pB) const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Point.inl>
