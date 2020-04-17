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


namespace corsika {


    template <typename dim>
    auto Vector<dim>::GetComponents() const
    {
    	return BaseVector<dim>::qVector;
    }


    template <typename dim>
    auto Vector<dim>::GetComponents(CoordinateSystem const& pCS) const
    {
      if (&pCS == BaseVector<dim>::cs) {
        return BaseVector<dim>::qVector;
      } else {
        return QuantityVector<dim>(
            getTransformation(*BaseVector<dim>::cs, pCS).linear() *
            BaseVector<dim>::qVector.eVector);
      }
    }

    template <typename dim>
    void Vector<dim>::rebase(CoordinateSystem const& pCS)
    {
      BaseVector<dim>::qVector = GetComponents(pCS);
      BaseVector<dim>::cs = &pCS;
    }

    template <typename dim>
    auto Vector<dim>::norm() const
    {
    	return BaseVector<dim>::qVector.norm();
    }

    template <typename dim>
    auto Vector<dim>::GetNorm() const
    {
    	return BaseVector<dim>::qVector.norm();
    }

    template <typename dim>
    auto Vector<dim>::squaredNorm() const
    {
    	return BaseVector<dim>::qVector.squaredNorm();
    }

    template <typename dim>
    auto Vector<dim>::GetSquaredNorm() const
    {
    	return BaseVector<dim>::qVector.squaredNorm();
    }

    template <typename dim>
    template <typename dim2>
    auto Vector<dim>::parallelProjectionOnto(Vector<dim2> const& pVec,
                                CoordinateSystem const& pCS) const
    {
      auto const ourCompVec = GetComponents(pCS);
      auto const otherCompVec = pVec.GetComponents(pCS);
      auto const& a = ourCompVec.eVector;
      auto const& b = otherCompVec.eVector;

      return Vector<dim>(pCS, QuantityVector<dim>(b * ((a.dot(b)) / b.squaredNorm())));
    }

    template <typename dim>
    template <typename dim2>
    auto Vector<dim>::parallelProjectionOnto(Vector<dim2> const& pVec) const
    {
      return parallelProjectionOnto<dim2>(pVec, *BaseVector<dim>::cs);
    }

    template <typename dim>
    auto Vector<dim>::operator+(Vector<dim> const& pVec) const
    {
      auto const components =
          GetComponents(*BaseVector<dim>::cs) + pVec.GetComponents(*BaseVector<dim>::cs);
      return Vector<dim>(*BaseVector<dim>::cs, components);
    }

    template <typename dim>
    auto Vector<dim>::operator-(Vector<dim> const& pVec) const
    {
      auto const components = GetComponents() - pVec.GetComponents(*BaseVector<dim>::cs);
      return Vector<dim>(*BaseVector<dim>::cs, components);
    }

    template <typename dim>
    auto&  Vector<dim>::operator*=(double const p)
	{
      BaseVector<dim>::qVector *= p;
      return *this;
    }

    template <typename dim>
    template <typename ScalarDim>
    auto  Vector<dim>::operator*(phys::units::quantity<ScalarDim, double> const p) const
    {
      using ProdDim = phys::units::detail::product_d<dim, ScalarDim>;

      return Vector<ProdDim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);
    }

    template <typename dim>
    template <typename ScalarDim>
    auto Vector<dim>::operator/(phys::units::quantity<ScalarDim, double> const p) const
    {
      return (*this) * (1 / p);
    }

    template <typename dim>
    auto Vector<dim>::operator*(double const p) const
    {
      return Vector<dim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);
    }

    template <typename dim>
    auto Vector<dim>::operator/(double const p) const
    {
      return Vector<dim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector / p);
    }

    template <typename dim>
    auto& Vector<dim>::operator+=(Vector<dim> const& pVec)
	{
      BaseVector<dim>::qVector += pVec.GetComponents(*BaseVector<dim>::cs);
      return *this;
    }

    template <typename dim>
    auto& Vector<dim>::operator-=(Vector<dim> const& pVec)
	{
      BaseVector<dim>::qVector -= pVec.GetComponents(*BaseVector<dim>::cs);
      return *this;
    }

    template <typename dim>
    auto& Vector<dim>::operator-() const
    {
      return Vector<dim>(*BaseVector<dim>::cs, -BaseVector<dim>::qVector);
    }

    template <typename dim>
    auto Vector<dim>::normalized() const
    {
    	return (*this) * (1 / norm());
    }

    template <typename dim>
    template <typename dim2>
    auto Vector<dim>::cross(Vector<dim2> pV) const
    {
      auto const c1 = GetComponents().eVector;
      auto const c2 = pV.GetComponents(*BaseVector<dim>::cs).eVector;
      auto const bareResult = c1.cross(c2);

      using ProdDim = phys::units::detail::product_d<dim, dim2>;
      return Vector<ProdDim>(*BaseVector<dim>::cs, bareResult);
    }

    template <typename dim>
    template <typename dim2>
    auto Vector<dim>::dot(Vector<dim2> pV) const
    {
      auto const c1 = GetComponents().eVector;
      auto const c2 = pV.GetComponents(*BaseVector<dim>::cs).eVector;
      auto const bareResult = c1.dot(c2);

      using ProdDim = phys::units::detail::product_d<dim, dim2>;

      return phys::units::quantity<ProdDim, double>(phys::units::detail::magnitude_tag,
                                                    bareResult);
    }


} // namespace corsika

