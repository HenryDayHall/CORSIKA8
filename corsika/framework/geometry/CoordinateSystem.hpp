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

#include <Eigen/Dense>
#include <stdexcept>
#include <memory>

namespace corsika {

  typedef Eigen::Transform<double, 3, Eigen::Affine> EigenTransform;
  typedef Eigen::Translation<double, 3> EigenTranslation;

  template <typename T>
  class Vector; // fwd decl

  class CoordinateSystem; // fwd decl
  /**
   * To refer to CoordinateSystems, only the CoordinateSystemPtr must be used.
   */
  using CoordinateSystemPtr = std::shared_ptr<CoordinateSystem const>;

  class RootCoordinateSystem;                             // fwd decl
  static CoordinateSystemPtr get_root_CoordinateSystem(); // fwd decl

  /**
   * A class to store the reference for a geometric object
   *
   * A CoordinateSystem can only be created in reference and relative
   * to other CoordinateSystem sytems. Thus, the geometric
   * transformation between all CoordinateSystems is known.
   *
   * Only the \sa RootCoordinateSystem can be created as a singleton
   * as global main reference point.
   *
   * Thus, new CoordinateSystems can only be created using
   * transformation: \sa rotateToZ, \sa rotate, \sa translateAndRotate
   * below.
   */

  class CoordinateSystem {

    /**
     * Constructor only from referenceCS, given the transformation matrix transf
     */
    CoordinateSystem(CoordinateSystemPtr referenceCS, EigenTransform const& transf)
        : referenceCS_(referenceCS)
        , transf_(transf) {}

    /**
     * for creating the root CS
     */
    CoordinateSystem()
        : referenceCS_(nullptr)
        , transf_(EigenTransform::Identity()) {}

  public:
    // default resource allocation
    CoordinateSystem(CoordinateSystem const&) = default;
    CoordinateSystem(CoordinateSystem&&) = default;
    CoordinateSystem& operator=(CoordinateSystem const& pCS) = default;
    ~CoordinateSystem() = default;

    inline CoordinateSystemPtr translate(QuantityVector<length_d> vector) const;

    /**
     * creates a new CS in which vVec points in direction of the new z-axis
     */
    template <typename TDim>
    CoordinateSystemPtr rotateToZ(Vector<TDim> vVec) const;

    template <typename TDim>
    CoordinateSystemPtr rotate(QuantityVector<TDim> axis, double angle) const;

    template <typename TDim>
    CoordinateSystemPtr translateAndRotate(QuantityVector<length_d> translation,
                                           QuantityVector<TDim> axis, double angle);

    inline CoordinateSystemPtr getReferenceCS() const;

    inline EigenTransform const& getTransform() const;

    inline bool operator==(CoordinateSystem const&) const;
    inline bool operator!=(CoordinateSystem const&) const;

  protected:
    static CoordinateSystem createCS() { return CoordinateSystem(); }

    friend CoordinateSystemPtr get_root_CoordinateSystem(); /// this is the only way to
    /// create ONE unique root CS

  private:
    std::shared_ptr<CoordinateSystem const> referenceCS_;
    EigenTransform transf_;
  };

  EigenTransform getTransformation(CoordinateSystemPtr c1, CoordinateSystemPtr c2);

} // namespace corsika

#include <corsika/detail/framework/geometry/CoordinateSystem.inl>
