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
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/logging/Logging.hpp>

#include <Eigen/Dense>

namespace corsika {

  /**
     This utility class handles Lorentz boost between different
     referenence frames, using FourVectors.
   */

  class COMBoost {

  public:
    //! construct a COMBoost given four-vector of projectile and mass of target
    COMBoost(FourVector<HEPEnergyType, Vector<hepmomentum_d>> const& Pprojectile,
             HEPEnergyType const massTarget);

    //! construct a COMBoost to boost into the rest frame given a 3-momentum and mass
    COMBoost(Vector<hepmomentum_d> const& momentum, HEPEnergyType mass);

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    template <typename FourVector>
    FourVector toCoM(FourVector const& p) const;

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    template <typename FourVector>
    FourVector fromCoM(FourVector const& p) const;

    CoordinateSystemPtr getRotatedCS() const;

  protected:
    void setBoost(double coshEta, double sinhEta);

  private:
    Eigen::Matrix2d boost_, inverseBoost_;
    CoordinateSystemPtr originalCS_, rotatedCS_;

  };
} // namespace corsika

#include <corsika/detail/framework/utility/COMBoost.inl>
