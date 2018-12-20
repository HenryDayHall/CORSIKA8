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

double constexpr absMargin = 1e-8;

TEST_CASE("boosts") {
  CoordinateSystem& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  // relativistic energy
  auto energy = [](MassType m, Vector<momentum_d> const& p) {
    return sqrt(m * m * cSquared * cSquared + p.squaredNorm() * cSquared);
  };

  // mandelstam-s
  auto s = [](EnergyType E, QuantityVector<momentum_d> const& p) {
    return E * E / cSquared - p.squaredNorm();
  };

  MassType const projectileMass = 4_GeV / cSquared;
  Vector<momentum_d> pProjectileLab{rootCS, {0_GeV / c, 100_GeV / c, 0_GeV / c}};
  EnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);

  MassType const targetMass = 1_GeV / cSquared;
  Vector<momentum_d> pTargetLab{rootCS, {0_Ns, 0_Ns, 0_Ns}};
  EnergyType const eTargetLab = energy(targetMass, pTargetLab);

  COMBoost boost(eProjectileLab, pProjectileLab, targetMass);

  auto const [eProjectileCoM, pProjectileCoM] =
      boost.toCoM(eProjectileLab, pProjectileLab);
  auto const [eTargetCoM, pTargetCoM] = boost.toCoM(eTargetLab, pTargetLab);

  auto const sumPCoM = pProjectileCoM + pTargetCoM;

  CHECK(s(eProjectileLab + eTargetLab,
          pProjectileLab.GetComponents() + pTargetLab.GetComponents()) /
            (1_GeV / c) / (1_GeV / c) ==
        Approx(s(eProjectileCoM + eTargetCoM, pProjectileCoM + pTargetCoM) / (1_GeV / c) /
               (1_GeV / c)));

  CHECK(sumPCoM[2] / (1_GeV / c) == Approx(0).margin(absMargin));

  auto const [eProjectileBack, pProjectileBack] =
      boost.fromCoM(eProjectileCoM, pProjectileCoM);

  CHECK(eProjectileBack / eProjectileLab == Approx(1));
  CHECK((pProjectileBack - pProjectileLab).norm() / (1_GeV / c) ==
        Approx(0).margin(absMargin));
}
