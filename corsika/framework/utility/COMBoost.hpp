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

#include <iostream>

#include <Eigen/Dense>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>
#include <corsika/framework/geometry/FourVector.hpp>

namespace corsika {

  /**
     This utility class handles Lorentz boost between different
     referenence frames, using FourVectors.
   */

  class COMBoost {
    Eigen::Matrix3d fRotation;
    Eigen::Matrix2d fBoost, fInverseBoost;
    corsika::CoordinateSystem const& fCS;

  public:
    //! construct a COMBoost given four-vector of prjectile and mass of target
    COMBoost(
        const corsika::FourVector<
            corsika::units::si::HEPEnergyType,
            corsika::Vector<corsika::units::si::hepmomentum_d>>& Pprojectile,
        const corsika::units::si::HEPEnergyType massTarget);

    inline auto const& GetRotationMatrix() const;

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    template <typename FourVector>
    inline FourVector toCoM(const FourVector& p) const ;

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    template <typename FourVector>
    inline FourVector fromCoM(const FourVector& p) const ;
  };
} // namespace corsika

#include <corsika/detail/framework/utility/COMBoost.inl>
