/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/ParticleCut.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <catch2/catch.hpp>
#include <corsika/setup/SetupStack.hpp>

using namespace corsika;
using namespace corsika::particle_cut;
using namespace corsika::units;
using namespace corsika::units::si;

TEST_CASE("ParticleCut", "[processes]") {
  feenableexcept(FE_INVALID);
  using EnvType = setup::Environment;

  EnvType env;
  const geometry::CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup empty particle stack
  setup::Stack stack;
  stack.Clear();
  // two energies
  const HEPEnergyType Eabove = 1_TeV;
  const HEPEnergyType Ebelow = 10_GeV;
  // list of arbitrary particles
  const std::vector<particles::Code> particleList = {
      particles::Code::PiPlus,   particles::Code::PiMinus, particles::Code::KPlus,
      particles::Code::KMinus,   particles::Code::K0Long,  particles::Code::K0Short,
      particles::Code::Electron, particles::Code::MuPlus,  particles::Code::NuE,
      particles::Code::Neutron,  particles::Code::NuMu};

  // common staring point
  const geometry::Point point0(rootCS, 0_m, 0_m, 0_m);

  SECTION("cut on particle type: inv") {

    ParticleCut cut(20_GeV, false, true);
    CHECK(cut.GetECut() == 20_GeV);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, Eabove,
            corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    corsika::SecondaryView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                         corsika::MomentumVector, geometry::Point,
                                         units::si::TimeType>{
          proType, Eabove, corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    CHECK(view.getEntries() == 9);
    CHECK(cut.GetNumberInvParticles() == 2);
    CHECK(cut.GetInvEnergy() / 1_GeV == 2000);
  }

  SECTION("cut on particle type: em") {

    ParticleCut cut(20_GeV, true, false);

    // add primary particle to stack
    auto particle = stack.AddParticle(std::make_tuple(
        particles::Code::Proton, Eabove,
        corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}), point0, 0_ns));
    // view on secondary particles
    corsika::setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList) {
      projectile.AddSecondary(std::make_tuple(
          proType, Eabove, corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          point0, 0_ns));
    }
    cut.DoSecondaries(view);

    CHECK(view.getEntries() == 10);
    CHECK(cut.GetNumberEmParticles() == 1);
    CHECK(cut.GetEmEnergy() / 1_GeV == 1000);
  }

  SECTION("cut low energy") {
    ParticleCut cut(20_GeV, true, true);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, Eabove,
            corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    corsika::SecondaryView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies below the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                         corsika::MomentumVector, geometry::Point,
                                         units::si::TimeType>{
          proType, Ebelow, corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    cut.DoSecondaries(view);

    CHECK(view.getEntries() == 1);
    CHECK(view.getSize() == 13);
  }

  SECTION("cut on time") {
    ParticleCut cut(20_GeV, false, false);
    const TimeType too_late = 1_s;

    // add primary particle to stack
    auto particle = stack.AddParticle(std::make_tuple(
        particles::Code::Proton, Eabove,
        corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}), point0, 1_ns));
    // view on secondary particles
    corsika::setup::StackView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList) {
      projectile.AddSecondary(std::make_tuple(
          proType, Eabove, corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          point0, too_late));
    }
    cut.DoSecondaries(view);

    CHECK(view.getEntries() == 0);
    CHECK(cut.GetCutEnergy() / 1_GeV == 11000);
    cut.Reset();
    CHECK(cut.GetCutEnergy() == 0_GeV);
  }

  corsika::setup::Trajectory const track = setup::testing::make_track<setup::Trajectory>(
      geometry::Line{point0,
                     geometry::Vector<units::si::SpeedType::dimension_type>{
                         rootCS, {0_m / second, 0_m / second, -units::constants::c}}},
      12_m / units::constants::c);

  SECTION("cut on DoContinous, just invisibles") {

    ParticleCut cut(20_GeV, false, true);
    CHECK(cut.GetECut() == 20_GeV);

    // add particles, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList) {
      auto particle = stack.AddParticle(std::make_tuple(
          proType, Eabove, corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          point0, 0_ns));
      cut.DoContinuous(particle, track);
    }

    CHECK(stack.getEntries() == 9);
    CHECK(cut.GetNumberInvParticles() == 2);
    CHECK(cut.GetInvEnergy() / 1_GeV == 2000);
  }
}
