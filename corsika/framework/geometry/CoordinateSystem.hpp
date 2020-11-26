n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
 * \file CoordinateSystem.hpp
 **/

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/logging/Logging.hpp>

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

  /// this is the only way to create ONE unique root CS
  static CoordinateSystemPtr get_root_CoordinateSystem(); 

  /**
   * Creates new CoordinateSystemPtr by translation along \a vector
   */
  inline CoordinateSystemPtr make_translation(CoordinateSystemPtr const& cs,
                                              QuantityVector<length_d> const& vector);

  /**
   * creates a new CoordinateSystem in which vVec points in direction of the new z-axis, \a vVec
   */
  template <typename TDim>
  inline CoordinateSystemPtr make_rotationToZ(CoordinateSystemPtr const& cs,
                                              Vector<TDim> const& vVec);

  /**
   * creates a new CoordinateSystem, rotated around axis by angle.
   */
  template <typename TDim>
  inline CoordinateSystemPtr make_rotation(CoordinateSystemPtr const& cs,
                                           QuantityVector<TDim> const& axis,
                                           double const angle);

  /**
   * creates a new CoordinateSystem, translated by \a translation and rotated around \a axis by \a angle.
   */
  template <typename TDim>
  inline CoordinateSystemPtr make_translationAndRotation(
      CoordinateSystemPtr const& cs, QuantityVector<length_d> const& translation,
      QuantityVector<TDim> const& axis, double const angle);

  /**
   * A class to store the reference coordinate system for a geometric object
   *
   * A CoordinateSystem can only be created in reference and relative
   * to other CoordinateSystems. Thus, the geometric
   * transformation between all CoordinateSystems is always known and stored.
   *
   * The static (singleton) function \ref make_root_CoordinateSystem is
   * the only way to create and access the global top-level
   * CoordinateSystem obect. CoordinateSystem objects should be
   * *abosulte* *only* handled in their form of CoordinateSystemPtr,
   * which are shared_ptr that handle the lifetime of the entire
   * CoordinateSystem hirarchy.
   *
   * Thus, new CoordinateSystem are only be created (via
   * CoordinateSystemPtr) by transforing existing CoordinateSystem
   * using: \ref make_rotationToZ, \ref make_rotation, or \ref
   * make_translationAndRotation, see below.
   *
   * Warning: As a consequence, never try to access, modify, copy, the raw
   * CoordinateSystem objects directly, this will almost certainly result in undefined
   * behaviour. Only access, copy, handle them via CoordinateSystemPtr.
   */

  class CoordinateSystem {

    /**
     * Constructor only from referenceCS, given the transformation matrix transf
     */
    CoordinateSystem(CoordinateSystemPtr const& referenceCS, EigenTransform const& transf)
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
    CoordinateSystem& operator=(CoordinateSystem const& pCS) =
        delete; // avoid making copies
    ~CoordinateSystem() = default;

    /**
     * Checks, if this is the unique ROOT CS
     */
    inline bool isRoot() const { return !referenceCS_; }

    inline CoordinateSystemPtr getReferenceCS() const;

    inline EigenTransform const& getTransform() const;

    inline bool operator==(CoordinateSystem const&) const;
    inline bool operator!=(CoordinateSystem const&) const;

  protected:
    static CoordinateSystem createCS() { return CoordinateSystem(); }

    /**
     * \name Friends
     * Manipulation and creation functions.
     * \{
     **/

    friend CoordinateSystemPtr get_root_CoordinateSystem();

    friend CoordinateSystemPtr make_translation(CoordinateSystemPtr const& cs,
                                                QuantityVector<length_d> const& vector);
    template <typename TDim>
    friend CoordinateSystemPtr make_rotationToZ(CoordinateSystemPtr const& cs,
                                                Vector<TDim> const& vVec);
    template <typename TDim>
    friend CoordinateSystemPtr make_rotation(CoordinateSystemPtr const& cs,
                                             QuantityVector<TDim> const& axis,
                                             double const angle);
    template <typename TDim>
    friend CoordinateSystemPtr make_translationAndRotation(
        CoordinateSystemPtr const& cs, QuantityVector<length_d> const& translation,
        QuantityVector<TDim> const& axis, double const angle);

    /** \} **/

  private:
    CoordinateSystemPtr referenceCS_;
    EigenTransform transf_;
  };

  /**
   * Transformation matrix from one reference system to another.
   *
   * returns the transformation matrix necessary to transform primitives with coordinates
   * in \a pFrom to \a pTo, e.g.
   * \f$ \vec{v}^{\text{(to)}} = \mathcal{M} \vec{v}^{\text{(from)}} \f$
   * (\f$ \vec{v}^{(.)} \f$ denotes the coordinates/components of the component in
   * the indicated CoordinateSystem).
   */
  inline EigenTransform get_transformation(CoordinateSystemPtr const& c1,
                                           CoordinateSystemPtr const& c2);

} // namespace corsika

#include <corsika/detail/framework/geometry/CoordinateSystem.inl>
