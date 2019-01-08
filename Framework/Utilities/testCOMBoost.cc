/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>
#include <iostream>

using namespace corsika::geometry;
using namespace corsika::utl;
using namespace corsika::units::si;
using corsika::units::constants::c;
using corsika::units::constants::cSquared;

double constexpr absMargin = 1e-6;

TEST_CASE("boosts") {
  CoordinateSystem& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  // relativistic energy
  auto energy = [](HEPMassType m, Vector<hepmomentum_d> const& p) {
    return sqrt(m * m + p.squaredNorm());
  };

  // mandelstam-s
  auto s = [](HEPEnergyType E, QuantityVector<hepmomentum_d> const& p) {
    return E * E - p.squaredNorm();
  };

  // define projectile kinematics in lab frame
  HEPMassType const projectileMass = 1._GeV;
  Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 1_PeV, 0_GeV}};
  HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);

  // define target kinematics in lab frame
  HEPMassType const targetMass = 1_GeV;
  Vector<hepmomentum_d> pTargetLab{rootCS, {0_eV, 0_eV, 0_eV}};
  HEPEnergyType const eTargetLab = energy(targetMass, pTargetLab);

  // define boost to com frame
  COMBoost boost(eProjectileLab, pProjectileLab, targetMass);

  // boost projecticle
  auto const [eProjectileCoM, pProjectileCoM] =
      boost.toCoM(eProjectileLab, pProjectileLab);

  // boost target
  auto const [eTargetCoM, pTargetCoM] = boost.toCoM(eTargetLab, pTargetLab);

  // sum of momenta in CoM, should be 0
  auto const sumPCoM = pProjectileCoM + pTargetCoM;
  CHECK(sumPCoM[2] / 1_GeV == Approx(0).margin(absMargin));

  // mandelstam-s should be invariant under transformation
  CHECK(s(eProjectileLab + eTargetLab,
          pProjectileLab.GetComponents() + pTargetLab.GetComponents()) /
            1_GeV / 1_GeV ==
        Approx(s(eProjectileCoM + eTargetCoM, pProjectileCoM + pTargetCoM) / 1_GeV /
               1_GeV));

  // boost back...
  auto const [eProjectileBack, pProjectileBack] =
      boost.fromCoM(eProjectileCoM, pProjectileCoM);

  // ...should yield original values before the boosts
  CHECK(eProjectileBack / eProjectileLab == Approx(1));
  CHECK((pProjectileBack - pProjectileLab).norm() / pProjectileLab.norm() ==
        Approx(0).margin(absMargin));
}
