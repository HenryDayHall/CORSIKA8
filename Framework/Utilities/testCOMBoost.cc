/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/geometry/FourVector.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

#include <Eigen/Dense>

#include <iostream>

using namespace corsika::geometry;
using namespace corsika::utl;
using namespace corsika::units::si;
using corsika::units::constants::c;
using corsika::units::constants::cSquared;

double constexpr absMargin = 1e-6;

CoordinateSystem const& rootCS =
    RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

// helper function for energy-momentum
// relativistic energy
auto const energy = [](HEPMassType m, Vector<hepmomentum_d> const& p) {
  return sqrt(m * m + p.squaredNorm());
};

// helper function for mandelstam-s
auto const s = [](HEPEnergyType E, QuantityVector<hepmomentum_d> const& p) {
  return E * E - p.squaredNorm();
};

TEST_CASE("rotation") {
  // define projectile kinematics in lab frame
  HEPMassType const projectileMass = 1_GeV;
  HEPMassType const targetMass = 1.0e300_eV;
  Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 0_PeV, 1_GeV}};
  HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
  const FourVector PprojLab(eProjectileLab, pProjectileLab);

  Eigen::Vector3d e1, e2, e3;
  e1 << 1, 0, 0;
  e2 << 0, 1, 0;
  e3 << 0, 0, 1;

  // define boost to com frame
  SECTION("pos. z-axis") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 0_GeV, 1_GeV}}}, targetMass);
    auto const& rot = boost.GetRotationMatrix();

    CHECK((rot * e3 - e3).norm() == Approx(0).margin(absMargin));
    CHECK((rot * e1).norm() == Approx(1));
    CHECK((rot * e2).norm() == Approx(1));
    CHECK((rot * e3).norm() == Approx(1));
    CHECK(rot.determinant() == Approx(1));
  }

  SECTION("y-axis in upper half") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 1_GeV, 1_meV}}}, targetMass);
    auto const& rot = boost.GetRotationMatrix();

    CHECK((rot * e2 - e3).norm() == Approx(0).margin(absMargin));
    CHECK((rot * e1).norm() == Approx(1));
    CHECK((rot * e2).norm() == Approx(1));
    CHECK((rot * e3).norm() == Approx(1));
    CHECK(rot.determinant() == Approx(1));
  }

  SECTION("x-axis in upper half") {
    COMBoost boost({eProjectileLab, {rootCS, {1_GeV, 0_GeV, 1_meV}}}, targetMass);
    auto const& rot = boost.GetRotationMatrix();

    CHECK((rot * e1 - e3).norm() == Approx(0).margin(absMargin));
    CHECK((rot * e1).norm() == Approx(1));
    CHECK((rot * e2).norm() == Approx(1));
    CHECK((rot * e3).norm() == Approx(1));
    CHECK(rot.determinant() == Approx(1));
  }

  SECTION("neg. z-axis") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 0_GeV, -1_GeV}}}, targetMass);
    auto const& rot = boost.GetRotationMatrix();

    CHECK((rot * (-e3) - e3).norm() == Approx(0).margin(absMargin));
    CHECK((rot * e1).norm() == Approx(1));
    CHECK((rot * e2).norm() == Approx(1));
    CHECK((rot * e3).norm() == Approx(1));
    CHECK(rot.determinant() == Approx(1));
  }

  SECTION("x-axis lower half") {
    COMBoost boost({eProjectileLab, {rootCS, {1_GeV, 0_GeV, -1_meV}}}, targetMass);
    auto const& rot = boost.GetRotationMatrix();

    CHECK((rot * e1 - e3).norm() == Approx(0).margin(absMargin));
    CHECK((rot * e1).norm() == Approx(1));
    CHECK((rot * e2).norm() == Approx(1));
    CHECK((rot * e3).norm() == Approx(1));
    CHECK(rot.determinant() == Approx(1));
  }

  SECTION("y-axis lower half") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 1_GeV, -1_meV}}}, targetMass);
    auto const& rot = boost.GetRotationMatrix();

    CHECK((rot * e2 - e3).norm() == Approx(0).margin(absMargin));
    CHECK((rot * e1).norm() == Approx(1));
    CHECK((rot * e2).norm() == Approx(1));
    CHECK((rot * e3).norm() == Approx(1));
    CHECK(rot.determinant() == Approx(1));
  }
}

TEST_CASE("boosts") {
  // define target kinematics in lab frame
  HEPMassType const targetMass = 1_GeV;
  Vector<hepmomentum_d> pTargetLab{rootCS, {0_eV, 0_eV, 0_eV}};
  HEPEnergyType const eTargetLab = energy(targetMass, pTargetLab);

  /*
    General tests check the interface and basic operation
   */

  SECTION("General tests") {

    // define projectile kinematics in lab frame
    HEPMassType const projectileMass = 1._GeV;
    Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 1_PeV, 0_GeV}};
    HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.GetSpaceLikeComponents() + PtargCoM.GetSpaceLikeComponents();
    CHECK(sumPCoM.norm() / 1_GeV == Approx(0).margin(absMargin));

    // mandelstam-s should be invariant under transformation
    CHECK(s(eProjectileLab + eTargetLab,
            pProjectileLab.GetComponents() + pTargetLab.GetComponents()) /
              1_GeV / 1_GeV ==
          Approx(s(PprojCoM.GetTimeLikeComponent() + PtargCoM.GetTimeLikeComponent(),
                   PprojCoM.GetSpaceLikeComponents().GetComponents() +
                       PtargCoM.GetSpaceLikeComponents().GetComponents()) /
                 1_GeV / 1_GeV));

    // boost back...
    auto const PprojBack = boost.fromCoM(PprojCoM);

    // ...should yield original values before the boosts
    CHECK(PprojBack.GetTimeLikeComponent() / PprojLab.GetTimeLikeComponent() ==
          Approx(1));
    CHECK(
        (PprojBack.GetSpaceLikeComponents() - PprojLab.GetSpaceLikeComponents()).norm() /
            PprojLab.GetSpaceLikeComponents().norm() ==
        Approx(0).margin(absMargin));
  }

  /*
    special case: projectile along -z
   */

  SECTION("Test boost along z-axis") {

    // define projectile kinematics in lab frame
    HEPMassType const projectileMass = 1_GeV;
    Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 0_PeV, -1_PeV}};
    HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.GetSpaceLikeComponents() + PtargCoM.GetSpaceLikeComponents();
    CHECK(sumPCoM.norm() / 1_GeV == Approx(0).margin(absMargin));
  }

  /*
    special case: projectile with arbitrary direction
   */

  SECTION("Test boost along tilted axis") {

    const HEPMomentumType P0 = 1_PeV;
    double theta = 33.;
    double phi = -10.;
    auto momentumComponents = [](double theta, double phi, HEPMomentumType ptot) {
      return std::make_tuple(ptot * sin(theta) * cos(phi), ptot * sin(theta) * sin(phi),
                             -ptot * cos(theta));
    };
    auto const [px, py, pz] =
        momentumComponents(theta / 180. * M_PI, phi / 180. * M_PI, P0);

    // define projectile kinematics in lab frame
    HEPMassType const projectileMass = 1_GeV;
    Vector<hepmomentum_d> pProjectileLab(rootCS, {px, py, pz});
    HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.GetSpaceLikeComponents() + PtargCoM.GetSpaceLikeComponents();
    CHECK(sumPCoM.norm() / 1_GeV == Approx(0).margin(absMargin));
  }

  /*
    test the ultra-high energy behaviour: E=ZeV
   */

  SECTION("High energy") {
    // define projectile kinematics in lab frame
    HEPMassType const projectileMass = 1_GeV;
    HEPMomentumType P0 = 1_ZeV;
    Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 0_PeV, -P0}};
    HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.GetSpaceLikeComponents() + PtargCoM.GetSpaceLikeComponents();
    CHECK(sumPCoM.norm() / P0 == Approx(0).margin(absMargin)); // MAKE RELATIVE CHECK
  }
}
