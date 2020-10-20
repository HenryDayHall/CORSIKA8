
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/particle_cut/ParticleCut.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <corsika/media/Environment.hpp>

#include <corsika/setup/SetupStack.hpp>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::particle_cut;
using namespace corsika::units::si;

TEST_CASE("ParticleCut", "[processes]") {
  feenableexcept(FE_INVALID);
  using EnvType = corsika::Environment<setup::IEnvironmentModel>;
  EnvType env;
  const corsika::CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup empty particle stack
  setup::Stack stack;
  stack.Clear();
  // two energies
  const HEPEnergyType Eabove = 1_TeV;
  const HEPEnergyType Ebelow = 10_GeV;
  // list of arbitrary particles
  std::vector<corsika::Code> particleList = {
      corsika::Code::PiPlus,   corsika::Code::PiMinus, corsika::Code::KPlus,
      corsika::Code::KMinus,   corsika::Code::K0Long,  corsika::Code::K0Short,
      corsika::Code::Electron, corsika::Code::MuPlus,  corsika::Code::NuE,
      corsika::Code::Neutron};

  SECTION("cut on particle type") {

    ParticleCut cut(20_GeV);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{
            corsika::Code::Proton, Eabove,
            corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            corsika::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    corsika::SecondaryView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(
          std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                     corsika::Point, units::si::TimeType>{
              proType, Eabove, corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
              corsika::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    cut.DoSecondaries(view);

    REQUIRE(view.GetSize() == 8);
  }

  SECTION("cut low energy") {
    ParticleCut cut(20_GeV);

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{
            corsika::Code::Proton, Eabove,
            corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            corsika::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    corsika::SecondaryView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies below the threshold
    // only cut is by species
    for (auto proType : particleList)
      projectile.AddSecondary(
          std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                     corsika::Point, units::si::TimeType>{
              proType, Ebelow, corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
              corsika::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});

    cut.DoSecondaries(view);

    REQUIRE(view.GetSize() == 0);
  }
}
