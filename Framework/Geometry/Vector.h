/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_VECTOR_H_
#define _include_VECTOR_H_

#include <corsika/geometry/BaseVector.h>
#include <corsika/geometry/QuantityVector.h>

#include <corsika/units/PhysicalUnits.h>

/*!
 * A Vector represents a 3-vector in Euclidean space. It is defined by components
 * given in a specific CoordinateSystem. It has a physical dimension ("unit")
 * as part of its type, so you cannot mix up e.g. electric with magnetic fields
 * (but you could calculate their cross-product to get an energy flux vector).
 *
 * When transforming coordinate systems, a Vector is subject to the rotational
 * part only and invariant under translations.
 */

namespace corsika::geometry {

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
    auto GetComponents() const { return BaseVector<dim>::qVector; }

    /*!
     * returns a QuantityVector with the components given in an arbitrary
     * CoordinateSystem
     */
    auto GetComponents(CoordinateSystem const& pCS) const {
      if (&pCS == BaseVector<dim>::cs) {
        return BaseVector<dim>::qVector;
      } else {
        return QuantityVector<dim>(
            CoordinateSystem::GetTransformation(*BaseVector<dim>::cs, pCS).linear() *
            BaseVector<dim>::qVector.eVector);
      }
    }

    /*!
     * transforms the Vector into another CoordinateSystem by changing
     * its components internally
     */
    void rebase(CoordinateSystem const& pCS) {
      BaseVector<dim>::qVector = GetComponents(pCS);
      BaseVector<dim>::cs = &pCS;
    }

    /*!
     * returns the norm/length of the Vector. Before using this method,
     * think about whether squaredNorm() might be cheaper for your computation.
     */
    auto norm() const { return BaseVector<dim>::qVector.norm(); }
    auto GetNorm() const { return BaseVector<dim>::qVector.norm(); }

    /*!
     * returns the squared norm of the Vector. Before using this method,
     * think about whether norm() might be cheaper for your computation.
     */
    auto squaredNorm() const { return BaseVector<dim>::qVector.squaredNorm(); }
    auto GetSquaredNorm() const { return BaseVector<dim>::qVector.squaredNorm(); }

    /*!
     * returns a Vector \f$ \vec{v}_{\parallel} \f$ which is the parallel projection
     * of this vector \f$ \vec{v}_1 \f$ along another Vector \f$ \vec{v}_2 \f$ given by
     *   \f[
     *     \vec{v}_{\parallel} = \frac{\vec{v}_1 \cdot \vec{v}_2}{\vec{v}_2^2} \vec{v}_2
     *   \f]
     */
    template <typename dim2>
    auto parallelProjectionOnto(Vector<dim2> const& pVec,
                                CoordinateSystem const& pCS) const {
      auto const ourCompVec = GetComponents(pCS);
      auto const otherCompVec = pVec.GetComponents(pCS);
      auto const& a = ourCompVec.eVector;
      auto const& b = otherCompVec.eVector;

      return Vector<dim>(pCS, QuantityVector<dim>(b * ((a.dot(b)) / b.squaredNorm())));
    }

    template <typename dim2>
    auto parallelProjectionOnto(Vector<dim2> const& pVec) const {
      return parallelProjectionOnto<dim2>(pVec, *BaseVector<dim>::cs);
    }

    auto operator+(Vector<dim> const& pVec) const {
      auto const components =
          GetComponents(*BaseVector<dim>::cs) + pVec.GetComponents(*BaseVector<dim>::cs);
      return Vector<dim>(*BaseVector<dim>::cs, components);
    }

    auto operator-(Vector<dim> const& pVec) const {
      auto const components = GetComponents() - pVec.GetComponents(*BaseVector<dim>::cs);
      return Vector<dim>(*BaseVector<dim>::cs, components);
    }

    auto& operator*=(double const p) {
      BaseVector<dim>::qVector *= p;
      return *this;
    }

    template <typename ScalarDim>
    auto operator*(phys::units::quantity<ScalarDim, double> const p) const {
      using ProdDim = phys::units::detail::product_d<dim, ScalarDim>;

      return Vector<ProdDim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);
    }

    template <typename ScalarDim>
    auto operator/(phys::units::quantity<ScalarDim, double> const p) const {
      return (*this) * (1 / p);
    }

    auto operator*(double const p) const {
      return Vector<dim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);
    }

    auto operator/(double const p) const {
      return Vector<dim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector / p);
    }

    auto& operator+=(Vector<dim> const& pVec) {
      BaseVector<dim>::qVector += pVec.GetComponents(*BaseVector<dim>::cs);
      return *this;
    }

    auto& operator-=(Vector<dim> const& pVec) {
      BaseVector<dim>::qVector -= pVec.GetComponents(*BaseVector<dim>::cs);
      return *this;
    }

    auto& operator-() const {
      return Vector<dim>(*BaseVector<dim>::cs, -BaseVector<dim>::qVector);
    }

    auto normalized() const { return (*this) * (1 / norm()); }

    template <typename dim2>
    auto cross(Vector<dim2> pV) const {
      auto const c1 = GetComponents().eVector;
      auto const c2 = pV.GetComponents(*BaseVector<dim>::cs).eVector;
      auto const bareResult = c1.cross(c2);

      using ProdDim = phys::units::detail::product_d<dim, dim2>;
      return Vector<ProdDim>(*BaseVector<dim>::cs, bareResult);
    }

    template <typename dim2>
    auto dot(Vector<dim2> pV) const {
      auto const c1 = GetComponents().eVector;
      auto const c2 = pV.GetComponents(*BaseVector<dim>::cs).eVector;
      auto const bareResult = c1.dot(c2);

      using ProdDim = phys::units::detail::product_d<dim, dim2>;

      return phys::units::quantity<ProdDim, double>(phys::units::detail::magnitude_tag,
                                                    bareResult);
    }
  };

} // namespace corsika::geometry

#endif
