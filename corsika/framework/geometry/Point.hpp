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
        Point(CoordinateSystemPtr const& pCS, QuantityVector<length_d> const& pQVector)
                : BaseVector<length_d>(pCS, pQVector) {}

        Point(CoordinateSystemPtr const& cs, LengthType x, LengthType y, LengthType z)
                : BaseVector<length_d>(cs, {x, y, z}) {}

        /** \todo TODO: this should be private or protected, we don NOT want to expose numbers
         * without reference to outside:
         */
        inline QuantityVector<length_d> const& getCoordinates() const;
        inline QuantityVector<length_d>& getCoordinates();

        /**
           this always returns a QuantityVector as triple

           \returns A value type QuantityVector, since it may have to create a temporary
           object to transform to pCS.
        **/
        inline QuantityVector<length_d> getCoordinates(CoordinateSystemPtr const& pCS) const;

        /**
         * this always returns a QuantityVector as triple
         *
         *  \returns A reference type QuantityVector&, but be aware, the underlying class data
         *   is actually transformed to pCS, if needed. Thus, there may be an implicit call to
         *   \ref rebase.
         **/
        inline QuantityVector<length_d>& getCoordinates(CoordinateSystemPtr const& pCS);

        /**
         * \name access coordinate components
         * \{
         *
         * Note, if you access components in a different CoordinateSystem
         * pCS than the stored data, internally a temporary object will be
         * created and destroyed each call. This can be avoided by using
         * \ref rebase first.
         **/
        inline LengthType getX(CoordinateSystemPtr const& pCS) const;
        inline LengthType getY(CoordinateSystemPtr const& pCS) const;
        inline LengthType getZ(CoordinateSystemPtr const& pCS) const;
        /** \} **/

        /*!
         * transforms the Point into another CoordinateSystem by changing its
         * coordinates interally
         */
        inline void rebase(CoordinateSystemPtr const& pCS);

        inline Point operator+(Vector<length_d> const& pVec) const;

        /*!
         * returns the distance Vector between two points
         */
        inline Vector<length_d> operator-(Point const& pB) const;
    };

    /*
     * calculates the distance between two points
     */
    inline LengthType distance(Point const& p1, Point const& p2);

} // namespace corsika

#include <corsika/detail/framework/geometry/Point.inl>