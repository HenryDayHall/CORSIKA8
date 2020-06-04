/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/on_shell_check/OnShellCheck.h>

#include <corsika/environment/Environment.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/CorsikaFenv.h>

#include <corsika/setup/SetupStack.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::on_shell_check;
using namespace corsika::units;
using namespace corsika::units::si;

TEST_CASE("OnShellCheck", "[processes]") {
  feenableexcept(FE_INVALID);
  using EnvType = environment::Environment<setup::IEnvironmentModel>;
  EnvType env;
  const geometry::CoordinateSystem& rootCS = env.GetCoordinateSystem();

  // setup empty particle stack
  setup::Stack stack;
  stack.Clear();
  // two energies
  const HEPEnergyType E = 10_GeV;
  // list of arbitrary particles
  std::array<particles::Code, 2> particleList = {particles::Code::PiPlus,
                                                 particles::Code::PiMinus};

  std::array<double, 2> mass_shifts = {1.1, 1.001};

  SECTION("check particle masses") {

    OnShellCheck check(1.e-2, 0.01);

    check.Init();

    // add primary particle to stack
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, E,
            corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, 0_GeV}),
            geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    // view on secondary particles
    corsika::stack::SecondaryView view(particle);
    // ref. to primary particle through the secondary view.
    // only this way the secondary view is populated
    auto projectile = view.GetProjectile();
    // add secondaries, all with energies above the threshold
    // only cut is by species
    int count = -1;
    for (auto proType : particleList) {
      count++;
      const auto pz = sqrt((E - particles::GetMass(proType) * mass_shifts[count]) *
                           (E + particles::GetMass(proType) * mass_shifts[count]));
      auto const momentum = corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, pz});
      projectile.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                         corsika::stack::MomentumVector, geometry::Point,
                                         units::si::TimeType>{
          proType, E, momentum, geometry::Point(rootCS, 0_m, 0_m, 0_m), 0_ns});
    }
    check.DoSecondaries(view);
    int i = -1;
    for (auto& p : view) {
      i++;
      auto const Plab = corsika::geometry::FourVector(p.GetEnergy(), p.GetMomentum());
      auto const m_kinetic = Plab.GetNorm();
      if (i == 0)
        REQUIRE(m_kinetic / particles::PiPlus::GetMass() == Approx(1));
      else
        REQUIRE_FALSE(m_kinetic / particles::PiMinus::GetMass() == Approx(1));
    }
  }
}
