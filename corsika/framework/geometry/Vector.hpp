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

/*!
 * A Vector represents a 3-vector in Euclidean space. It is defined by components
 * given in a specific CoordinateSystem. It has a physical dimension ("unit")
 * as part of its type, so you cannot mix up e.g. electric with magnetic fields
 * (but you could calculate their cross-product to get an energy flux vector).
 *
 * When transforming coordinate systems, a Vector is subject to the rotational
 * part only and invariant under translations.
 */

namespace corsika {

  template <typename dim>
  class Vector : public BaseVector<dim> {
  public:
    using Quantity = phys::units::quantity<dim, double>;

  public:
    Vector(CoordinateSystem const& pCS, QuantityVector<dim> pQVector)
        : BaseVector<dim>(pCS, pQVector) {}

    Vector(CoordinateSystem const& cs, Quantity x, Quantity y, Quantity z)
        : BaseVector<dim>(cs, QuantityVector<dim>(x, y, z)) {}

    /*!
     * returns a QuantityVector with the components given in the "home"
     * CoordinateSystem of the Vector
     */
    auto GetComponents() const;

    /*!
     * returns a QuantityVector with the components given in an arbitrary
     * CoordinateSystem
     */
    auto GetComponents(CoordinateSystem const& pCS) const;

    /*!
     * transforms the Vector into another CoordinateSystem by changing
     * its components internally
     */
    void rebase(CoordinateSystem const& pCS);

    /*!
     * returns the norm/length of the Vector. Before using this method,
     * think about whether squaredNorm() might be cheaper for your computation.
     */
    auto norm() const;

    auto GetNorm() const;

    /*!
     * returns the squared norm of the Vector. Before using this method,
     * think about whether norm() might be cheaper for your computation.
     */
    auto squaredNorm() const;

    auto GetSquaredNorm() const;
    /*!
     * returns a Vector \f$ \vec{v}_{\parallel} \f$ which is the parallel projection
     * of this vector \f$ \vec{v}_1 \f$ along another Vector \f$ \vec{v}_2 \f$ given by
     *   \f[
     *     \vec{v}_{\parallel} = \frac{\vec{v}_1 \cdot \vec{v}_2}{\vec{v}_2^2} \vec{v}_2
     *   \f]
     */
    template <typename dim2>
    auto parallelProjectionOnto(Vector<dim2> const& pVec,
                                CoordinateSystem const& pCS) const;
    template <typename dim2>
    auto parallelProjectionOnto(Vector<dim2> const& pVec) const;

    auto operator+(Vector<dim> const& pVec) const;

    auto operator-(Vector<dim> const& pVec) const;

    auto& operator*=(double const p);

    template <typename ScalarDim>
    auto operator*(phys::units::quantity<ScalarDim, double> const p) const;
    template <typename ScalarDim>
    auto operator/(phys::units::quantity<ScalarDim, double> const p) const;

    auto operator*(double const p) const;

    auto operator/(double const p) const;

    auto& operator+=(Vector<dim> const& pVec);

    auto& operator-=(Vector<dim> const& pVec);

    auto& operator-() const;

    auto normalized() const;

    template <typename dim2>
    auto cross(Vector<dim2> pV) const;

    template <typename dim2>
    auto dot(Vector<dim2> pV) const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Vector.inl>
