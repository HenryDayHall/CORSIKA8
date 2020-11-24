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
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <Eigen/Dense>

#include <memory>
#include <stdexcept>

namespace corsika {

  inline CoordinateSystemPtr CoordinateSystem::translate(
      QuantityVector<length_d> vector) const {
    EigenTransform const translation{EigenTranslation(vector.eigenVector_)};

    return std::make_shared<CoordinateSystem const>(*(new CoordinateSystem(
        std::make_shared<CoordinateSystem const>(*this), translation)));
  }

  template <typename TDim>
  CoordinateSystemPtr CoordinateSystem::rotateToZ(Vector<TDim> vVec) const {
    auto const a = vVec.normalized()
                       .getComponents(std::make_shared<CoordinateSystem const>(*this))
                       .getEigenVector();
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

    return std::make_shared<CoordinateSystem const>(*(new CoordinateSystem(
        std::make_shared<CoordinateSystem const>(*this), EigenTransform(A + B))));
  }

  template <typename TDim>
  CoordinateSystemPtr CoordinateSystem::rotate(QuantityVector<TDim> axis,
                                               double angle) const {
    if (axis.eigenVector_.isZero()) {
      throw std::runtime_error("null-vector given as axis parameter");
    }

    EigenTransform const rotation{
        Eigen::AngleAxisd(angle, axis.eigenVector_.normalized())};

    return std::make_shared<CoordinateSystem const>(
        CoordinateSystem(std::make_shared<CoordinateSystem const>(*this), rotation));
  }

  template <typename TDim>
  CoordinateSystemPtr CoordinateSystem::translateAndRotate(
      QuantityVector<length_d> translation, QuantityVector<TDim> axis, double angle) {
    if (axis.eigenVector_.isZero()) {
      throw std::runtime_error("null-vector given as axis parameter");
    }

    EigenTransform const transf{Eigen::AngleAxisd(angle, axis.eigenVector_.normalized()) *
                                EigenTranslation(translation.eigenVector_)};

    return std::make_shared<CoordinateSystem const>(CoordinateSystem(*this, transf));
  }

  CoordinateSystemPtr CoordinateSystem::getReferenceCS() const {
    return referenceCS_; //*(referenceCS_.get());
  }

  EigenTransform const& CoordinateSystem::getTransform() const { return transf_; }

  inline bool CoordinateSystem::operator==(CoordinateSystem const& cs) const {
    return referenceCS_ == cs.referenceCS_ && transf_.matrix() == cs.transf_.matrix();
  }

  inline bool CoordinateSystem::operator!=(CoordinateSystem const& cs) const {
    return !(cs == *this);
  }

  /**
   * returns the transformation matrix necessary to transform primitives with coordinates
   * in \a pFrom to \a pTo, e.g.
   * \f$ \vec{v}^{\text{(to)}} = \mathcal{M} \vec{v}^{\text{(from)}} \f$
   * (\f$ \vec{v}^{(.)} \f$ denotes the coordinates/components of the component in
   * the indicated CoordinateSystem).
   */
  inline EigenTransform getTransformation(CoordinateSystemPtr pFrom,
                                          CoordinateSystemPtr pTo) {
    CoordinateSystemPtr a{pFrom};
    CoordinateSystemPtr b{pTo};
    CoordinateSystemPtr commonBase{nullptr};

    while (a != b && b != nullptr) {
      a = pFrom;

      while (a != b && a != nullptr) { a = a->getReferenceCS(); }

      if (a == b) break;

      b = b->getReferenceCS();
    }

    if (a == b && a != nullptr) {
      commonBase = a;

    } else {
      throw std::runtime_error("no connection between coordinate systems found!");
    }

    EigenTransform t = EigenTransform::Identity();
    CoordinateSystemPtr p = pFrom;

    while ((*p) != (*commonBase)) {
      t = p->getTransform() * t;
      p = p->getReferenceCS();
    }

    p = pTo;

    while (*p != *commonBase) {
      t = t * p->getTransform().inverse(Eigen::TransformTraits::Isometry);
      p = p->getReferenceCS();
    }

    return t;
  }

} // namespace corsika
