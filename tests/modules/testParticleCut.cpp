/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/ParticleCut.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/framework/process/ContinuousProcessIndex.hpp>

#include <SetupTestStack.hpp>
#include <SetupTestTrajectory.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("ParticleCut", "process,continuous,secondary") {

  logging::set_level(logging::level::info);

  feenableexcept(FE_INVALID);
  using EnvType = setup::Environment;

  EnvType env;
  CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();

  // setup empty particle stack
  setup::Stack stack;
  stack.clear();
  // two energies
  HEPEnergyType const Eabove = 1_TeV;
  HEPEnergyType const Ebelow = 10_GeV;
  // list of arbitrary particles
  std::vector<Code> const particleList = {Code::PiPlus,   Code::PiMinus, Code::KPlus,
                                          Code::KMinus,   Code::K0Long,  Code::K0Short,
                                          Code::Electron, Code::MuPlus,  Code::NuE,
                                          Code::Neutron,  Code::NuMu};

  // common stating point
  const Point point0(rootCS, 0_m, 0_m, 0_m);

  SECTION("cut on particle type: inv") {

    // particle cut with 20GeV threshold for all, also cut invisible
    ParticleCut cut(20_GeV, false, true);
    CHECK(cut.getHadronKineticECut() == 20_GeV);

    // add primary particle to stack
    auto particle = stack.addParticle(
        std::make_tuple(Code::Proton, Eabove, DirectionVector(rootCS, {1, 0, 0}),
                        Point(rootCS, 0_m, 0_m, 0_m), 0_ns));
    // view on secondary particles
    setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.getProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.addSecondary(
          std::make_tuple(proType, Eabove, DirectionVector(rootCS, {1, 0, 0})));
    CHECK(view.getEntries() == 11);
    CHECK(stack.getEntries() == 12);

    cut.doSecondaries(view);

    CHECK(view.getEntries() == 9);
    CHECK(cut.getNumberInvParticles() == 2);
    CHECK(cut.getInvEnergy() / 1_GeV == Approx(2000.));
  }

  SECTION("cut on particle type: em") {

    ParticleCut cut(20_GeV, true, false);

    // add primary particle to stack
    auto particle = stack.addParticle(std::make_tuple(
        Code::Proton, Eabove, DirectionVector(rootCS, {1, 0, 0}), point0, 0_ns));
    // view on secondary particles
    setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.getProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList) {
      projectile.addSecondary(
          std::make_tuple(proType, Eabove, DirectionVector(rootCS, {1, 0, 0})));
    }
    cut.doSecondaries(view);

    CHECK(view.getEntries() == 10);
    CHECK(cut.getNumberEmParticles() == 1);
    CHECK(cut.getEmEnergy() / 1_GeV == 1000.);
  }

  SECTION("cut low energy") {
    ParticleCut cut(20_GeV, true, true);

    // add primary particle to stack
    auto particle = stack.addParticle(std::make_tuple(
        Code::Proton, Eabove, DirectionVector(rootCS, {1, 0, 0}), point0, 0_ns));
    // view on secondary particles
    setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.getProjectile();
    // add secondaries, all with energies below the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.addSecondary(
          std::make_tuple(proType, Ebelow, DirectionVector(rootCS, {1, 0, 0})));
    unsigned short A = 18;
    unsigned short Z = 8;
    projectile.addSecondary(std::make_tuple(get_nucleus_code(A, Z), Eabove * A,
                                            DirectionVector(rootCS, {1, 0, 0})));
    projectile.addSecondary(std::make_tuple(get_nucleus_code(A, Z), Ebelow * A,
                                            DirectionVector(rootCS, {1, 0, 0})));

    cut.doSecondaries(view);

    CHECK(view.getEntries() == 1);
    CHECK(view.getSize() == 13);
  }

  SECTION("cut low energy: electrons, photons, hadrons and muons") {
    ParticleCut cut(5_MeV, 5_MeV, 5_GeV, 5_GeV, true);

    // add primary particle to stack
    auto particle = stack.addParticle(std::make_tuple(Code::Proton, Eabove - Proton::mass,
                                                      DirectionVector(rootCS, {1, 0, 0}),
                                                      point0, 0_ns));
    // view on secondary particles
    setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.getProjectile();
    // add secondaries
    projectile.addSecondary(
        std::make_tuple(Code::Photon, 3_MeV, DirectionVector(rootCS, {1, 0, 0})));
    projectile.addSecondary(
        std::make_tuple(Code::Electron, 3_MeV, DirectionVector(rootCS, {1, 0, 0})));
    projectile.addSecondary(
        std::make_tuple(Code::PiPlus, 4_GeV, DirectionVector(rootCS, {1, 0, 0})));

    unsigned short A = 18;
    unsigned short Z = 8;
    projectile.addSecondary(std::make_tuple(get_nucleus_code(A, Z), 4_GeV * A,
                                            DirectionVector(rootCS, {1, 0, 0})));
    projectile.addSecondary(std::make_tuple(get_nucleus_code(A, Z), 6_GeV * A,
                                            DirectionVector(rootCS, {1, 0, 0})));

    cut.doSecondaries(view);

    CHECK(view.getEntries() == 1);
    CHECK(view.getSize() == 5);
  }

  SECTION("cut low energy:  reset thresholds of arbitrary set of particles") {
    ParticleCut cut({{Code::Electron, 5_MeV}, {Code::Positron, 50_MeV}}, false, true);
    CHECK(calculate_kinetic_energy_threshold(Code::Electron) !=
          calculate_kinetic_energy_threshold(Code::Positron));
    CHECK_FALSE(calculate_kinetic_energy_threshold(Code::Electron) == Electron::mass);
    // test default values still correct
    CHECK(calculate_kinetic_energy_threshold(Code::Proton) == 5_GeV);
  }

  SECTION("cut on time") {
    ParticleCut cut(20_GeV, false, false);
    const TimeType too_late = 1_s;

    // add primary particle to stack
    auto particle = stack.addParticle(std::make_tuple(
        Code::Proton, Eabove, DirectionVector(rootCS, {1, 0, 0}), point0, too_late));
    // view on secondary particles
    setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.getProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by time
    for (auto proType : particleList) {
      projectile.addSecondary(
          std::make_tuple(proType, Eabove, DirectionVector(rootCS, {1, 0, 0})));
    }
    cut.doSecondaries(view);

    CHECK(view.getEntries() == 0);
    CHECK(cut.getTimeCutEnergy() == 11 * Eabove);
    CHECK(cut.getCutEnergy() == 0_GeV);
    cut.reset();
    CHECK(cut.getTimeCutEnergy() == 0_GeV);
  }

  setup::Trajectory const track = setup::testing::make_track<setup::Trajectory>(
      Line{point0, VelocityVector{rootCS, {0_m / second, 0_m / second, -constants::c}}},
      12_m / constants::c);

  SECTION("cut on DoContinous, just invisibles") {

    ParticleCut cut(20_GeV, false, true);
    CHECK(cut.getHadronKineticECut() == 20_GeV);

    // add particles, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList) {
      auto particle = stack.addParticle(
          std::make_tuple(proType, Eabove - get_mass(proType),
                          DirectionVector(rootCS, {1, 0, 0}), point0, 0_ns));

      if (cut.doContinuous(particle, track) == ProcessReturn::ParticleAbsorbed) {
        particle.erase();
      }
    }

    CHECK(stack.getEntries() == 9);
    CHECK(cut.getNumberInvParticles() == 2);
    CHECK(cut.getInvEnergy() / 1_GeV == 2000);
  }
}
