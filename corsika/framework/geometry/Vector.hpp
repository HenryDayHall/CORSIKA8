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

namespace corsika {

  /*!
   * A Vector represents a 3-vector in Euclidean space.
   *
   * It is defined by components
   * given in a specific CoordinateSystem. It has a physical dimension ("unit")
   * as part of its type, so you cannot mix up e.g. electric with magnetic fields
   * (but you could calculate their cross-product to get an energy flux vector).
   *
   * When transforming coordinate systems, a Vector is subject to the rotational
   * part only and invariant under translations.
   */

  template <typename TDimension>
  class Vector : public BaseVector<TDimension> {
  public:
    using quantity_type = phys::units::quantity<TDimension, double>;
    using quantity_square_type =
        decltype(std::declval<quantity_type>() * std::declval<quantity_type>());

    Vector(CoordinateSystemPtr const& pCS, QuantityVector<TDimension> const& pQVector)
        : BaseVector<TDimension>(pCS, pQVector) {}

    Vector(CoordinateSystemPtr const& cs, quantity_type const x, quantity_type const y,
           quantity_type const z)
        : BaseVector<TDimension>(cs, QuantityVector<TDimension>(x, y, z)) {}

    /*!
     * \returns a QuantityVector with the components given in the "home"
     * CoordinateSystem of the Vector
     *
     * \todo this should best be protected, we don't want users to use
     * bare coordinates without reference frame
     */
    QuantityVector<TDimension> const& getComponents() const;
    QuantityVector<TDimension>& getComponents();

    /*!
     * returns a QuantityVector with the components given in an arbitrary
     * CoordinateSystem
     */
    QuantityVector<TDimension> getComponents(CoordinateSystemPtr const& pCS) const;

    /**
     * this always returns a QuantityVector as triple
     *
     *  \return A reference type QuantityVector&, but be aware, the underlying class data
     *   is actually transformed to pCS, if needed. Thus, there may be an implicit call to
     *   \ref rebase.
     **/
    QuantityVector<TDimension>& getComponents(CoordinateSystemPtr const& pCS);

    /**
     * \name Access coordinate components
     *
     * Note, if you access components in a different CoordinateSystem
     * pCS than the stored data, internally a temporary object will be
     * created and destroyed each call. This can be avoided by using
     * \ref rebase first.
     *
     * \{
     **/

    quantity_type getX(CoordinateSystemPtr const& pCS) const;
    quantity_type getY(CoordinateSystemPtr const& pCS) const;
    quantity_type getZ(CoordinateSystemPtr const& pCS) const;
    /** \} **/

    /*!
     * transforms the Vector into another CoordinateSystem by changing
     * its components internally
     */
    void rebase(CoordinateSystemPtr const& pCS);

    /*!
     * returns the norm/length of the Vector. Before using this method,
     * think about whether squaredNorm() might be cheaper for your computation.
     */
    quantity_type getNorm() const;

    /*!
     * returns the squared norm of the Vector. Before using this method,
     * think about whether norm() might be cheaper for your computation.
     */
    quantity_square_type getSquaredNorm() const;

    /*!
     * returns a Vector \f$ \vec{v}_{\parallel} \f$ which is the parallel projection
     * of this vector \f$ \vec{v}_1 \f$ along another Vector \f$ \vec{v}_2 \f$ given by
     *   \f[
     *     \vec{v}_{\parallel} = \frac{\vec{v}_1 \cdot \vec{v}_2}{\vec{v}_2^2} \vec{v}_2
     *   \f]
     */
    template <typename TDimension2>
    auto getParallelProjectionOnto(Vector<TDimension2> const& pVec,
                                   CoordinateSystemPtr const& pCS) const;
    template <typename TDimension2>
    auto getParallelProjectionOnto(Vector<TDimension2> const& pVec) const;

    Vector operator+(Vector<TDimension> const& pVec) const;

    Vector operator-(Vector<TDimension> const& pVec) const;

    auto& operator*=(double const p);

    template <typename TScalarDim>
    auto operator*(phys::units::quantity<TScalarDim, double> const p) const;
    template <typename TScalarDim>
    auto operator/(phys::units::quantity<TScalarDim, double> const p) const;

    auto operator*(double const p) const;

    auto operator/(double const p) const;

    auto& operator+=(Vector<TDimension> const& pVec);

    auto& operator-=(Vector<TDimension> const& pVec);

    auto& operator-() const;

    auto normalized() const;

    template <typename TDimension2>
    auto cross(Vector<TDimension2> const& pV) const;

    template <typename TDimension2>
    auto dot(Vector<TDimension2> const& pV) const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Vector.inl>
