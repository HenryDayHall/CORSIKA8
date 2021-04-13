/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <Eigen/Dense>

namespace corsika {

  /**
     @defgroup Utilities

     Collection of classes and methods to perform recurring tasks.
   **/

  /**
     @class
     @ingroup Utilities

     This utility class handles Lorentz boost between different
     referenence frames, using FourVector.

     The class is initialized with projectile and optionally target
     energy/momentum data. During initialization, a rotation matrix is
     calculated to represent the projectile movement (and thus the
     boost) along the z-axis. Also the inverse of this rotation is
     calculated. The Lorentz boost matrix and its inverse are
     determined as 2x2 matrices considering the energy and
     pz-momentum.

     Different constructors are offered with different specialization
     for the cases of collisions (projectile-target) or just decays
     (projectile only).
   */

  class COMBoost {

  public:
    //! construct a COMBoost given four-vector of projectile and mass of target (target at
    //! rest)
    COMBoost(FourVector<HEPEnergyType, MomentumVector> const& Pprojectile,
             HEPEnergyType const massTarget);

    //! construct a COMBoost to boost into the rest frame given a 3-momentum and mass
    COMBoost(MomentumVector const& momentum, HEPEnergyType mass);

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    template <typename FourVector>
    FourVector toCoM(FourVector const& p) const;

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    template <typename FourVector>
    FourVector fromCoM(FourVector const& p) const;

    //! returns the rotated coordinate system
    CoordinateSystemPtr getRotatedCS() const;

  protected:
    //! internal method
    void setBoost(double coshEta, double sinhEta);

  private:
    Eigen::Matrix2d boost_;
    Eigen::Matrix2d inverseBoost_;
    CoordinateSystemPtr originalCS_;
    CoordinateSystemPtr rotatedCS_;
  };
} // namespace corsika

#include <corsika/detail/framework/utility/COMBoost.inl>
