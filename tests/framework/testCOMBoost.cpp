/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

using namespace corsika;

double constexpr absMargin = 1e-6;

CoordinateSystemPtr rootCS = get_root_CoordinateSystem();

/**
 * \todo such helper functions should be moved to the FourVector class:
 **/
// helper function for energy-momentum
// relativistic energy
auto const energy = [](HEPMassType m, Vector<hepmomentum_d> const& p) {
  return sqrt(m * m + p.getSquaredNorm());
};

auto const momentum = [](HEPEnergyType E, HEPMassType m) { return sqrt(E * E - m * m); };

// helper function for mandelstam-s
auto const s = [](HEPEnergyType E, QuantityVector<hepmomentum_d> const& p) {
  return E * E - p.getSquaredNorm();
};

TEST_CASE("rotation") {
  // define projectile kinematics in lab frame
  HEPMassType const projectileMass = 1_GeV;
  HEPMassType const targetMass = 1.0e300_eV;
  Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 0_PeV, 1_GeV}};
  HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
  FourVector const PprojLab(eProjectileLab, pProjectileLab);

  Vector<hepenergy_d> e1(rootCS, {1_GeV, 0_GeV, 0_GeV});
  Vector<hepenergy_d> e2(rootCS, {0_GeV, 1_GeV, 0_GeV});
  Vector<hepenergy_d> e3(rootCS, {0_GeV, 0_GeV, 1_GeV});

  // define boost to com frame
  SECTION("pos. z-axis") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 0_GeV, 1_GeV}}}, targetMass);
    CoordinateSystemPtr rotCS = boost.getRotatedCS();

    e1.rebase(rotCS);
    e2.rebase(rotCS);
    e3.rebase(rotCS);
    
    // length of e1, e2 and e3 must all be 1_GeV in rotated CS (not boosted!)
    CHECK(e1.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e2.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e3.getNorm() / 1_GeV == Approx(1).margin(absMargin));

    // z-axis is along z-boost
    CHECK(e3.getX(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e3.getY(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e3.getZ(rotCS) / 1_GeV == Approx(1).margin(absMargin));
  }

  SECTION("y-axis in upper half") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 1_GeV, 1_meV}}}, targetMass);
    CoordinateSystemPtr rotCS = boost.getRotatedCS();

    e1.rebase(rotCS);
    e2.rebase(rotCS);
    e3.rebase(rotCS);

    // length of e1, e2 and e3 must all be 1_GeV in rotated CS (not boosted!)
    CHECK(e1.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e2.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e3.getNorm() / 1_GeV == Approx(1).margin(absMargin));

    // z-axis is along y-boost
    CHECK(e2.getX(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e2.getY(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e2.getZ(rotCS) / 1_GeV == Approx(1).margin(absMargin));
  }

  SECTION("x-axis in upper half") {
    COMBoost boost({eProjectileLab, {rootCS, {1_GeV, 0_GeV, 1_meV}}}, targetMass);
    CoordinateSystemPtr rotCS = boost.getRotatedCS();

    e1.rebase(rotCS);
    e2.rebase(rotCS);
    e3.rebase(rotCS);

    // length of e1, e2 and e3 must all be 1_GeV in rotated CS (not boosted!)
    CHECK(e1.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e2.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e3.getNorm() / 1_GeV == Approx(1).margin(absMargin));

    // z-axis is along x-boost
    CHECK(e1.getX(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e1.getY(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e1.getZ(rotCS) / 1_GeV == Approx(1).margin(absMargin));
  }

  SECTION("neg. z-axis") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 0_GeV, -1_GeV}}}, targetMass);
    CoordinateSystemPtr rotCS = boost.getRotatedCS();

    e1.rebase(rotCS);
    e2.rebase(rotCS);
    e3.rebase(rotCS);

    // length of e1, e2 and e3 must all be 1_GeV in rotated CS (not boosted!)
    CHECK(e1.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e2.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e3.getNorm() / 1_GeV == Approx(1).margin(absMargin));

    // z-axis is along -z-boost
    CHECK(-e3.getX(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(-e3.getY(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(-e3.getZ(rotCS) / 1_GeV == Approx(1).margin(absMargin));
  }

  SECTION("x-axis lower half") {
    COMBoost boost({eProjectileLab, {rootCS, {1_GeV, 0_GeV, -1_meV}}}, targetMass);
    CoordinateSystemPtr rotCS = boost.getRotatedCS();

    e1.rebase(rotCS);
    e2.rebase(rotCS);
    e3.rebase(rotCS);

    // length of e1, e2 and e3 must all be 1_GeV in rotated CS (not boosted!)
    CHECK(e1.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e2.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e3.getNorm() / 1_GeV == Approx(1).margin(absMargin));

    // z-axis is along x-boost
    CHECK(e1.getX(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e1.getY(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e1.getZ(rotCS) / 1_GeV == Approx(1).margin(absMargin));
  }

  SECTION("y-axis lower half") {
    COMBoost boost({eProjectileLab, {rootCS, {0_GeV, 1_GeV, -1_meV}}}, targetMass);
    CoordinateSystemPtr rotCS = boost.getRotatedCS();

    e1.rebase(rotCS);
    e2.rebase(rotCS);
    e3.rebase(rotCS);

    // length of e1, e2 and e3 must all be 1_GeV in rotated CS (not boosted!)
    CHECK(e1.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e2.getNorm() / 1_GeV == Approx(1).margin(absMargin));
    CHECK(e3.getNorm() / 1_GeV == Approx(1).margin(absMargin));

    // z-axis is along y-boost
    CHECK(e2.getX(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e2.getY(rotCS) / 1_GeV == Approx(0).margin(absMargin));
    CHECK(e2.getZ(rotCS) / 1_GeV == Approx(1).margin(absMargin));
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
    Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 20_GeV, 0_GeV}};
    HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
    FourVector const PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.getSpaceLikeComponents() + PtargCoM.getSpaceLikeComponents();
    CHECK(sumPCoM.getNorm() / 1_GeV == Approx(0).margin(absMargin));

    // mandelstam-s should be invariant under transformation
    CHECK(s(eProjectileLab + eTargetLab,
            pProjectileLab.getComponents() + pTargetLab.getComponents()) /
              1_GeV / 1_GeV ==
          Approx(s(PprojCoM.getTimeLikeComponent() + PtargCoM.getTimeLikeComponent(),
                   PprojCoM.getSpaceLikeComponents().getComponents() +
                       PtargCoM.getSpaceLikeComponents().getComponents()) /
                 1_GeV / 1_GeV));

    // boost back...
    auto const PprojBack = boost.fromCoM(PprojCoM);

    // ...should yield original values before the boosts
    CHECK(PprojBack.getTimeLikeComponent() / PprojLab.getTimeLikeComponent() ==
          Approx(1));
    CHECK((PprojBack.getSpaceLikeComponents() - PprojLab.getSpaceLikeComponents())
                  .getNorm() /
              PprojLab.getSpaceLikeComponents().getNorm() ==
          Approx(0).margin(absMargin));
  }

  /*
    special case: projectile along -z
   */

  SECTION("Test boost along z-axis") {

    // define projectile kinematics in lab frame
    HEPMassType const projectileMass = 1_GeV;
    Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, 0_GeV, -20_GeV}};
    HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
    FourVector const PprojLab(eProjectileLab, pProjectileLab);

    auto const sqrt_s_lab =
        sqrt(s(eProjectileLab + targetMass, pProjectileLab.GetComponents(rootCS)));

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);
    auto const a = PprojCoM.GetSpaceLikeComponents().GetComponents(boost.GetRotatedCS());
    CHECK(a.GetX() / 1_GeV == Approx(0));
    CHECK(a.GetY() / 1_GeV == Approx(0));
    CHECK(a.GetZ() / (momentum(sqrt_s_lab / 2, projectileMass)) == Approx(1));

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));
    CHECK(PtargCoM.GetTimeLikeComponent() / sqrt_s_lab == Approx(.5));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.getSpaceLikeComponents() + PtargCoM.getSpaceLikeComponents();
    CHECK(sumPCoM.getNorm() / 1_GeV == Approx(0).margin(absMargin));
  }

  /*
    special case: projectile with arbitrary direction
   */

  SECTION("Test boost along tilted axis") {

    HEPMomentumType const P0 = 1_PeV;
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
    FourVector const PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.getSpaceLikeComponents() + PtargCoM.getSpaceLikeComponents();
    CHECK(sumPCoM.getNorm() / 1_GeV == Approx(0).margin(absMargin));
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
    FourVector const PprojLab(eProjectileLab, pProjectileLab);

    // define boost to com frame
    COMBoost boost(PprojLab, targetMass);

    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(FourVector(targetMass, pTargetLab));

    // sum of momenta in CoM, should be 0
    auto const sumPCoM =
        PprojCoM.getSpaceLikeComponents() + PtargCoM.getSpaceLikeComponents();
    CHECK(sumPCoM.getNorm() / P0 == Approx(0).margin(absMargin)); // MAKE RELATIVE CHECK
  }
}

TEST_CASE("rest frame") {
  HEPMassType const projectileMass = 1_GeV;
  HEPMomentumType const P0 = 1_TeV;
  Vector<hepmomentum_d> pProjectileLab{rootCS, {0_GeV, P0, 0_GeV}};
  HEPEnergyType const eProjectileLab = energy(projectileMass, pProjectileLab);
  const FourVector PprojLab(eProjectileLab, pProjectileLab);

  COMBoost boostRest(pProjectileLab, projectileMass);
  auto const& csPrime = boostRest.GetRotatedCS();
  FourVector const rest4Mom = boostRest.toCoM(PprojLab);

  CHECK(rest4Mom.GetTimeLikeComponent() / 1_GeV == Approx(projectileMass / 1_GeV));
  CHECK(rest4Mom.GetSpaceLikeComponents().norm() / 1_GeV == Approx(0).margin(absMargin));

  FourVector const a{0_eV, Vector{csPrime, 0_eV, 5_GeV, 0_eV}};
  FourVector const b{0_eV, Vector{rootCS, 3_GeV, 0_eV, 0_eV}};
  auto const aLab = boostRest.fromCoM(a);
  auto const bLab = boostRest.fromCoM(b);

  CHECK(aLab.GetNorm() / a.GetNorm() == Approx(1));
  CHECK(aLab.GetSpaceLikeComponents().GetComponents(csPrime)[1].magnitude() ==
        Approx((5_GeV).magnitude()));
  CHECK(bLab.GetSpaceLikeComponents().GetComponents(rootCS)[0].magnitude() ==
        Approx((3_GeV).magnitude()));
}
