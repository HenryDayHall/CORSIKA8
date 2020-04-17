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


    // TODO: this should be private or protected, we don NOT want to expose numbers
    // without reference to outside:
    auto Point::GetCoordinates() const
    {
    	return BaseVector<length_d>::qVector;
    }

    auto Point::GetX() const
    {
    	return BaseVector<length_d>::qVector.GetX();
    }

    auto Point::GetY() const
    {
    	return BaseVector<length_d>::qVector.GetY();
    }

    auto Point::GetZ() const
    {
    	return BaseVector<length_d>::qVector.GetZ();
    }

    /// this always returns a QuantityVector as triple
    auto Point::GetCoordinates( CoordinateSystem const& pCS) const
    {
      if (&pCS == BaseVector<length_d>::cs) {
        return BaseVector<length_d>::qVector;
      } else {
        return QuantityVector<length_d>(
            getTransformation(*BaseVector<length_d>::cs, pCS) *
            BaseVector<length_d>::qVector.eVector);
      }
    }

    /*!
     * transforms the Point into another CoordinateSystem by changing its
     * coordinates interally
     */
    void Point::rebase(CoordinateSystem const& pCS)
    {
      BaseVector<length_d>::qVector = GetCoordinates(pCS);
      BaseVector<length_d>::cs = &pCS;
    }

    Point Point::operator+(Vector<length_d> const& pVec) const
    {
      return Point(*BaseVector<length_d>::cs,
                   GetCoordinates() + pVec.GetComponents(*BaseVector<length_d>::cs));
    }

    /*!
     * returns the distance Vector between two points
     */
    Vector<length_d> Point::operator-( Point const& pB) const
    {
      auto& cs = *BaseVector<length_d>::cs;
      return Vector<length_d>(cs, GetCoordinates() - pB.GetCoordinates(cs));
    }


} // namespace corsika

