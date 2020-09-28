/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/particle_cut/ParticleCut.h>

#include <corsika/environment/Environment.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/CorsikaFenv.h>

#include <corsika/setup/SetupStack.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::particle_cut;
using namespace corsika::units;
using namespace corsika::units::si;

TEST_CASE("ParticleCut", "[processes]") {
  feenableexcept(FE_INVALID);
  using EnvType = environment::Environment<setup::IEnvironmentModel>;
  EnvType env;
  const geometry::CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup empty particle stack
  setup::Stack stack;
  stack.Clear();
  // two energies
  const HEPEnergyType Eabove = 1_TeV;
  const HEPEnergyType Ebelow = 10_GeV;
  // list of arbitrary particles
  std::vector<particles::Code> particleList = {
      particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
      particles::Code::KMinus, particles::Code::K0Long, particles::Code::K0Short,
      particles::Code::Electron, particles::Code::MuPlus, particles::Code::NuE,
      particles::Code::Neutron};

  SECTION("cut on particle type: inv") {

    ParticleCut cut(20_GeV, false, true);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, Eabove,
            corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    setup::StackView view{particle};
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                         corsika::stack::MomentumVector, geometry::Point,
                                         units::si::TimeType>{
          proType, Eabove, corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    cut.DoSecondaries(view);

    CHECK(view.GetSize() == 9);
  }

  SECTION("cut on particle type: em") {

    ParticleCut cut(20_GeV, true, false);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, Eabove,
            corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    corsika::stack::SecondaryView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                         corsika::stack::MomentumVector, geometry::Point,
                                         units::si::TimeType>{
          proType, Eabove, corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    cut.DoSecondaries(view);

    CHECK(view.GetSize() == 9);
  }

  SECTION("cut low energy") {
    ParticleCut cut(20_GeV, true, true);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, Eabove,
            corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    setup::StackView view{particle};
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies below the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                         corsika::stack::MomentumVector, geometry::Point,
                                         units::si::TimeType>{
          proType, Ebelow, corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
          geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    cut.DoSecondaries(view);

    REQUIRE(view.getEntries() == 0);
    REQUIRE(view.getSize() == 10);
  }
}
