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
    using MomentumVector = corsika::geometry::Vector<units::si::momentum_d>;

  public:
    //! construct a COMBoost given energy and momentum of projectile and mass of target
    COMBoost(units::si::EnergyType eProjectile, MomentumVector const& pProjectile,
             units::si::MassType mTarget);

    //! transforms a 4-momentum from lab frame to the center-of-mass frame
    std::tuple<units::si::EnergyType, geometry::QuantityVector<units::si::momentum_d>>
    toCoM(units::si::EnergyType E, MomentumVector p) const;

    //! transforms a 4-momentum from the center-of-mass frame back to lab frame
    std::tuple<units::si::EnergyType, MomentumVector> fromCoM(
        units::si::EnergyType E,
        geometry::QuantityVector<units::si::momentum_d> pCoM) const;
  };
} // namespace corsika::utl

#endif
