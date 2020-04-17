/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <corsika/process/ObservationPlane.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using namespace corsika::units::si;
using namespace corsika::observation_plane;
using namespace corsika;
using namespace corsika;
using namespace corsika;

TEST_CASE("ContinuousProcess interface", "[proccesses][observation_plane]") {

  auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  /*
    Test with downward going 1_GeV neutrino, starting at 0, 1_m, 10m

    ObservationPlane has origin at 0,0,0
   */

  auto [stack, viewPtr] =
      setup::testing::setupStack(particles::Code::NuE, 0, 0, 1_GeV, nodePtr, cs);
  [[maybe_unused]] setup::StackView& view = *viewPtr;
  auto particle = stack->GetNextParticle();

  Point const start(cs, {0_m, 1_m, 10_m});
  Vector<units::si::SpeedType::dimension_type> vec(cs, 0_m / second, 0_m / second,
                                                   -units::constants::c);
  Line line(start, vec);
  Trajectory<Line> track(line, 12_m / units::constants::c);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  {
    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt((Elab - m) * (Elab + m));
    };
    stack.AddParticle(
        std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector, Point,
                   units::si::TimeType>{
            Code::NuMu, 1_GeV,
            corsika::MomentumVector(
                rootCS, {0_GeV, 0_GeV, -elab2plab(1_GeV, NuMu::GetMass())}),
            Point(rootCS, {1_m, 1_m, 10_m}), 0_ns});
  }
  auto particle = stack.GetNextParticle();

  SECTION("horizontal plane") {

    Plane const obsPlane(Point(cs, {0_m, 0_m, 0_m}),
                         Vector<dimensionless_d>(cs, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, Vector<dimensionless_d>(cs, {1., 0., 0.}),
                         "particles.dat", true);

    const LengthType length = obs.MaxStepLength(particle, track);
    const process::EProcessReturn ret = obs.DoContinuous(particle, track);

    REQUIRE(length / 10_m == Approx(1).margin(1e-4));
    REQUIRE(ret == process::EProcessReturn::eParticleAbsorbed);
  }

  SECTION("inclined plane") {}

  SECTION("transparent plane") {
    Plane const obsPlane(Point(cs, {0_m, 0_m, 0_m}),
                         Vector<dimensionless_d>(cs, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, Vector<dimensionless_d>(cs, {1., 0., 0.}),
                         "particles.dat", false);

    const LengthType length = obs.MaxStepLength(particle, track);
    const process::EProcessReturn ret = obs.DoContinuous(particle, track);

    REQUIRE(length / 10_m == Approx(1).margin(1e-4));
    REQUIRE(ret == process::EProcessReturn::eOk);
  }
}
