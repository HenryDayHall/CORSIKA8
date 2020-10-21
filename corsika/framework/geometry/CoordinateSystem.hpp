n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/utility/sgn.hpp>

#include <Eigen/Dense>
#include <stdexcept>

/*
 * FIXME Review this global typedef.
 */
typedef Eigen::Transform<double, 3, Eigen::Affine> EigenTransform;
typedef Eigen::Translation<double, 3> EigenTranslation;

namespace corsika {

  class RootCoordinateSystem;

  template <typename T>
  class Vector;

  class CoordinateSystem {

    CoordinateSystem const* reference = nullptr;
    EigenTransform transf;

    CoordinateSystem(CoordinateSystem const& reference, EigenTransform const& transf)
        : reference(&reference)
        , transf(transf) {}

    CoordinateSystem()
        : // for creating the root CS
        transf(EigenTransform::Identity()) {}

  public:
    // FIXME missing test for self assignment
    inline CoordinateSystem& operator=(const CoordinateSystem& pCS);

    inline CoordinateSystem translate(QuantityVector<length_d> vector) const;

    /**
     * creates a new CS in which vVec points in direction of the new z-axis
     */
    template <typename TDim>
    auto RotateToZ(Vector<TDim> vVec) const;

    template <typename TDim>
    auto rotate(QuantityVector<TDim> axis, double angle) const;

    template <typename TDim>
    auto translateAndRotate(QuantityVector<length_d> translation,
                            QuantityVector<TDim> axis, double angle);

    inline CoordinateSystem const* GetReference() const;

    inline const EigenTransform& GetTransform() const;

  protected:
    static CoordinateSystem CreateCS() { return CoordinateSystem(); }

    friend corsika::RootCoordinateSystem; /// this is the only class that can
                                          /// create ONE unique root CS
  };

  EigenTransform getTransformation(CoordinateSystem const& c1,
                                   CoordinateSystem const& c2);

} // namespace corsika

#include <corsika/detail/framework/geometry/CoordinateSystem.inl>
