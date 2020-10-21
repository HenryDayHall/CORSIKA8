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

#include <corsika/framework/geometry/QuantityVector.hpp>
#include <Eigen/Dense>
#include <stdexcept>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/sgn.hpp>


namespace corsika {


    CoordinateSystem& CoordinateSystem::operator=(const corsika::CoordinateSystem& pCS)
    {
      reference = pCS.reference;
      transf = pCS.transf;
      return *this;
    }

    inline CoordinateSystem CoordinateSystem::translate(QuantityVector<length_d> vector) const
    {
      EigenTransform const translation{EigenTranslation(vector.eVector)};

      return CoordinateSystem(*this, translation);
    }

    template <typename TDim>
    auto CoordinateSystem::RotateToZ(Vector<TDim> vVec) const
    {
      auto const a = vVec.normalized().GetComponents(*this).eVector;
      auto const a1 = a(0), a2 = a(1);

      auto const s = corsika::sgn(a(2));
      auto const c = 1 / (1 + s * a(2));

      Eigen::Matrix3d A, B;

      if (s > 0) {
        A << 1, 0, -a1,                     // comment to prevent clang-format
            0, 1, -a2,                      // .
            a1, a2, 1;                      // .
        B << -a1 * a1 * c, -a1 * a2 * c, 0, // .
            -a1 * a2 * c, -a2 * a2 * c, 0,  // .
            0, 0, -(a1 * a1 + a2 * a2) * c; // .

      } else {
        A << 1, 0, a1,                      // .
            0, -1, -a2,                     // .
            a1, a2, -1;                     // .
        B << -a1 * a1 * c, -a1 * a2 * c, 0, // .
            +a1 * a2 * c, +a2 * a2 * c, 0,  // .
            0, 0, (a1 * a1 + a2 * a2) * c;  // .
      }

      return CoordinateSystem(*this, EigenTransform(A + B));
    }

    template <typename TDim>
    auto CoordinateSystem::rotate(QuantityVector<TDim> axis, double angle) const
    {
      if (axis.eVector.isZero()) {
        throw std::runtime_error("null-vector given as axis parameter");
      }

      EigenTransform const rotation{Eigen::AngleAxisd(angle, axis.eVector.normalized())};

      return CoordinateSystem(*this, rotation);
    }

    template <typename TDim>
    auto CoordinateSystem::translateAndRotate(QuantityVector<length_d> translation, QuantityVector<TDim> axis, double angle)
    {
      if (axis.eVector.isZero()) {
        throw std::runtime_error("null-vector given as axis parameter");
      }

      EigenTransform const transf{Eigen::AngleAxisd(angle, axis.eVector.normalized()) *
                                  EigenTranslation(translation.eVector)};

      return CoordinateSystem(*this, transf);
    }

    CoordinateSystem const* CoordinateSystem::GetReference() const
    {
    	return reference;
    }

    const EigenTransform& CoordinateSystem::GetTransform() const
    {
    	return transf;
    }

    /**
     * returns the transformation matrix necessary to transform primitives with coordinates
     * in \a pFrom to \a pTo, e.g.
     * \f$ \vec{v}^{\text{(to)}} = \mathcal{M} \vec{v}^{\text{(from)}} \f$
     * (\f$ \vec{v}^{(.)} \f$ denotes the coordinates/components of the component in
     * the indicated CoordinateSystem).
     */
    inline EigenTransform getTransformation(CoordinateSystem const& pFrom,
                                                                                          CoordinateSystem const& pTo) {
      CoordinateSystem const* a{&pFrom};
      CoordinateSystem const* b{&pTo};
      CoordinateSystem const* commonBase{nullptr};

      while (a != b && b != nullptr) {
        a = &pFrom;

        while (a != b && a != nullptr) { a = a->GetReference(); }

        if (a == b) break;

        b = b->GetReference();
      }

      if (a == b && a != nullptr) {
        commonBase = a;

      } else {
        throw std::runtime_error("no connection between coordinate systems found!");
      }

      EigenTransform t = EigenTransform::Identity();
      auto* p = &pFrom;

      while (p != commonBase) {
        t = p->GetTransform() * t;
        p = p->GetReference();
      }

      p = &pTo;

      while (p != commonBase) {
        t = t * p->GetTransform().inverse(Eigen::TransformTraits::Isometry);
        p = p->GetReference();
      }

      return t;
    }

} // namespace corsika

