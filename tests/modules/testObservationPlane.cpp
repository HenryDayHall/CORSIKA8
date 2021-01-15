/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/modules/ObservationPlane.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <SetupTestEnvironment.hpp>
#include <SetupTestStack.hpp>
#include <SetupTestTrajectory.hpp>

using namespace corsika;

TEST_CASE("ContinuousProcess interface", "[proccesses][observation_plane]") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  /*
    Test with downward going 1_GeV neutrino, starting at 0,1_m,10m

    ObservationPlane has origin at 0,0,0
   */
  auto [stack, viewPtr] =
      setup::testing::setup_stack(Code::NuE, 0, 0, 1_GeV, nodePtr, cs);
  [[maybe_unused]] setup::StackView& view = *viewPtr;
  auto particle = stack->getNextParticle();

  Point const start(cs, {0_m, 1_m, 10_m});
  VelocityVector vec(cs, 0_m / second, 0_m / second, -constants::c);
  Line line(start, vec);

  setup::Trajectory track =
      setup::testing::make_track<setup::Trajectory>(line, 12_m / constants::c);

  particle.setPosition(Point(cs, {1_m, 1_m, 10_m})); // moving already along -z

  SECTION("horizontal plane") {

    Plane const obsPlane(Point(cs, {0_m, 0_m, 0_m}), DirectionVector(cs, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, DirectionVector(cs, {1., 0., 0.}), "particles.dat",
                         true);

    LengthType const length = obs.getMaxStepLength(particle, track);
    ProcessReturn const ret = obs.doContinuous(particle, track);

    CHECK(length / 10_m == Approx(1).margin(1e-4));
    CHECK(ret == ProcessReturn::ParticleAbsorbed);
  }

  SECTION("inclined plane") {}

  SECTION("transparent plane") {
    Plane const obsPlane(Point(cs, {0_m, 0_m, 0_m}), DirectionVector(cs, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, DirectionVector(cs, {1., 0., 0.}), "particles.dat",
                         false);

    LengthType const length = obs.getMaxStepLength(particle, track);
    ProcessReturn const ret = obs.doContinuous(particle, track);

    CHECK(length / 10_m == Approx(1).margin(1e-4));
    CHECK(ret == ProcessReturn::Ok);
  }
}
