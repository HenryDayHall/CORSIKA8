/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <Eigen/Dense>

#include <memory>
#include <stdexcept>

namespace corsika {

  inline CoordinateSystemPtr CoordinateSystem::getReferenceCS() const {
    return referenceCS_;
  }

  inline EigenTransform const& CoordinateSystem::getTransform() const { return transf_; }

  inline bool CoordinateSystem::operator==(CoordinateSystem const& cs) const {
    return referenceCS_ == cs.referenceCS_ && transf_.matrix() == cs.transf_.matrix();
  }

  inline bool CoordinateSystem::operator!=(CoordinateSystem const& cs) const {
    return !(cs == *this);
  }

  /// find transformation between two CS, using most optimal common base
  inline EigenTransform get_transformation(CoordinateSystem const& pFrom,
                                           CoordinateSystem const& pTo) {
    CoordinateSystem const* a{&pFrom};
    CoordinateSystem const* b{&pTo};

    while (a != b && b) {

      // traverse pFrom
      a = &pFrom;
      while (a != b && a) { a = a->getReferenceCS().get(); }

      if (a == b) break;

      b = b->getReferenceCS().get();
    }

    if (a != b || a == nullptr) {
      throw std::runtime_error("no connection between coordinate systems found!");
    }

    CoordinateSystem const* commonBase = a;
    CoordinateSystem const* p = &pFrom;
    EigenTransform t = EigenTransform::Identity();
    while ((*p) != (*commonBase)) {
      t = p->getTransform() * t;
      p = p->getReferenceCS().get();
    }

    p = &pTo;

    while (*p != *commonBase) {
      t = t * p->getTransform().inverse(Eigen::TransformTraits::Isometry);
      p = p->getReferenceCS().get();
    }

    return t;
  }

  inline CoordinateSystemPtr make_translation(CoordinateSystemPtr const& cs,
                                              QuantityVector<length_d> const& vector) {
    EigenTransform const translation{EigenTranslation(vector.getEigenVector())};
    return CoordinateSystemPtr{new CoordinateSystem(cs, translation)};
  }

  template <typename TDim>
  inline CoordinateSystemPtr make_rotationToZ(CoordinateSystemPtr const& cs,
                                              Vector<TDim> const& vVec) {
    auto const a = vVec.normalized().getComponents(cs).getEigenVector();
    auto const a1 = a(0), a2 = a(1), a3 = a(2);

    Eigen::Matrix3d A, B;

    if (a3 > 0) {
      auto const c = 1 / (1 + a3);
      A << 1, 0, a1,                      // comment to prevent clang-format
          0, 1, a2,                       // .
          -a1, -a2, 1;                    // .
      B << -a1 * a1 * c, -a1 * a2 * c, 0, // .
          -a1 * a2 * c, -a2 * a2 * c, 0,  // .
          0, 0, -(a1 * a1 + a2 * a2) * c; // .

    } else {
      auto const c = 1 / (1 - a3);
      A << 1, 0, a1,                      // .
          0, -1, a2,                      // .
          a1, -a2, -1;                    // .
      B << -a1 * a1 * c, +a1 * a2 * c, 0, // .
          -a1 * a2 * c, +a2 * a2 * c, 0,  // .
          0, 0, (a1 * a1 + a2 * a2) * c;  // .
    }

    return CoordinateSystemPtr{new CoordinateSystem{cs, EigenTransform{A + B}}};
  }

  template <typename TDim>
  inline CoordinateSystemPtr make_rotation(CoordinateSystemPtr const& cs,
                                           QuantityVector<TDim> const& axis,
                                           double const angle) {
    if (axis.getEigenVector().isZero()) {
      throw std::runtime_error("null-vector given as axis parameter");
    }

    EigenTransform const rotation{
        Eigen::AngleAxisd(angle, axis.getEigenVector().normalized())};

    return CoordinateSystemPtr{new CoordinateSystem{cs, rotation}};
  }

  template <typename TDim>
  inline CoordinateSystemPtr make_translationAndRotation(
      CoordinateSystemPtr const& cs, QuantityVector<length_d> const& translation,
      QuantityVector<TDim> const& axis, double const angle) {
    if (axis.getEigenVector().isZero()) {
      throw std::runtime_error("null-vector given as axis parameter");
    }

    EigenTransform const transf{
        Eigen::AngleAxisd(angle, axis.getEigenVector().normalized()) *
        EigenTranslation(translation.getEigenVector())};

    return CoordinateSystemPtr{new CoordinateSystem{cs, transf}};
  }

} // namespace corsika
