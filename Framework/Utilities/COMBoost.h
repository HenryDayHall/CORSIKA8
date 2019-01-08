/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_utilties_comboost_h_
#define _include_corsika_utilties_comboost_h_

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/QuantityVector.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <Eigen/Dense>
#include <tuple>

namespace corsika::utl {
  class COMBoost {
    Eigen::Matrix3d fRotation;
    Eigen::Matrix2d fBoost, fInverseBoost;
    corsika::geometry::CoordinateSystem const& fCS;
    using MomentumVector = corsika::geometry::Vector<units::si::hepmomentum_d>;

  public:
    //! construct a COMBoost given energy and momentum of projectile and mass of target
    COMBoost(units::si::HEPEnergyType eProjectile, MomentumVector const& pProjectile,
             units::si::HEPMassType mTarget);

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    std::tuple<units::si::HEPEnergyType,
               geometry::QuantityVector<units::si::hepmomentum_d>>
    toCoM(units::si::HEPEnergyType E, MomentumVector p) const;

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    std::tuple<units::si::HEPEnergyType, MomentumVector> fromCoM(
        units::si::HEPEnergyType E,
        geometry::QuantityVector<units::si::hepmomentum_d> pCoM) const;
  };
} // namespace corsika::utl

#endif
