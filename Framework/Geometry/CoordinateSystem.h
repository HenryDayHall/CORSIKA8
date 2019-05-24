/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_COORDINATESYSTEM_H_
#define _include_COORDINATESYSTEM_H_

#include <corsika/geometry/QuantityVector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/sgn.h>
#include <Eigen/Dense>
#include <stdexcept>

typedef Eigen::Transform<double, 3, Eigen::Affine> EigenTransform;
typedef Eigen::Translation<double, 3> EigenTranslation;

namespace corsika::geometry {

  class RootCoordinateSystem;
  template <typename T>
  class Vector;

  using corsika::units::si::length_d;

  class CoordinateSystem {
    CoordinateSystem const* reference = nullptr;
    EigenTransform transf;

    CoordinateSystem(CoordinateSystem const& reference, EigenTransform const& transf)
        : reference(&reference)
        , transf(transf) {}

    CoordinateSystem()
        : // for creating the root CS
        transf(EigenTransform::Identity()) {}

  protected:
    static auto CreateCS() { return CoordinateSystem(); }
    friend corsika::geometry::RootCoordinateSystem; /// this is the only class that can
                                                    /// create ONE unique root CS

  public:
    static EigenTransform GetTransformation(CoordinateSystem const& c1,
                                            CoordinateSystem const& c2);

    auto& operator=(const CoordinateSystem& pCS) {
      reference = pCS.reference;
      transf = pCS.transf;
      return *this;
    }

    auto translate(QuantityVector<length_d> vector) const {
      EigenTransform const translation{EigenTranslation(vector.eVector)};

      return CoordinateSystem(*this, translation);
    }

    template <typename TDim>
    auto RotateToZ(Vector<TDim> vVec) const {
      auto const a = vVec.normalized().GetComponents(*this).eVector;
      auto const a1 = a(0), a2 = a(1);

      auto const s = utl::sgn(a(2));
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
    auto rotate(QuantityVector<TDim> axis, double angle) const {
      if (axis.eVector.isZero()) {
        throw std::runtime_error("null-vector given as axis parameter");
      }

      EigenTransform const rotation{Eigen::AngleAxisd(angle, axis.eVector.normalized())};

      return CoordinateSystem(*this, rotation);
    }

    template <typename TDim>
    auto translateAndRotate(QuantityVector<phys::units::length_d> translation,
                            QuantityVector<TDim> axis, double angle) {
      if (axis.eVector.isZero()) {
        throw std::runtime_error("null-vector given as axis parameter");
      }

      EigenTransform const transf{Eigen::AngleAxisd(angle, axis.eVector.normalized()) *
                                  EigenTranslation(translation.eVector)};

      return CoordinateSystem(*this, transf);
    }

    auto const* GetReference() const { return reference; }

    auto const& GetTransform() const { return transf; }
  };

} // namespace corsika::geometry

#endif
