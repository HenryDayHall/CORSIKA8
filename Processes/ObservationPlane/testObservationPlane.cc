/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
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

#include <corsika/process/observation_plane/ObservationPlane.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika::units::si;
using namespace corsika::process::observation_plane;
using namespace corsika;
using namespace corsika::geometry;
using namespace corsika::particles;

TEST_CASE("ContinuousProcess interface", "[proccesses][observation_plane]") {

  auto const& rootCS = RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  /*
    Test with downward going 1_GeV neutrino, starting at 0,1_m,10m

    ObservationPlane has origin at 0,0,0
   */

  Point const start(rootCS, {0_m, 1_m, 10_m});
  Vector<units::si::SpeedType::dimension_type> vec(rootCS, 0_m / second, 0_m / second,
                                                   -units::constants::c);
  Line line(start, vec);
  Trajectory<Line> track(line, 10_m / units::constants::c);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  {
    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt((Elab - m) * (Elab + m));
    };
    stack.AddParticle(
        std::tuple<Code, units::si::HEPEnergyType, corsika::stack::MomentumVector, Point,
                   units::si::TimeType>{
            Code::NuMu, 1_GeV,
            corsika::stack::MomentumVector(
                rootCS, {0_GeV, 0_GeV, -elab2plab(1_GeV, NuMu::GetMass())}),
            Point(rootCS, {1_m, 1_m, 10_m}), 0_ns});
  }
  auto particle = stack.GetNextParticle();

  SECTION("horizontal plane") {

    Plane const obsPlane(Point(rootCS, {0_m, 0_m, 0_m}),
                         Vector<dimensionless_d>(rootCS, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, "particles.dat");

    obs.Init();
    [[maybe_unused]] const LengthType length = obs.MaxStepLength(particle, track);
    [[maybe_unused]] const process::EProcessReturn ret =
        obs.DoContinuous(particle, track);

    SECTION("steplength") { REQUIRE(length == 10_m); }

    /*
    SECTION("horizontal plane") {
      REQUIRE(true); // todo: we have to check content of output file...

    }
    */
  }

  SECTION("inclined plane") {
    Plane const obsPlane(Point(rootCS, {0_m, 0_m, 0_m}),
                         Vector<dimensionless_d>(rootCS, {1., 1., 0.5}));
    ObservationPlane obs(obsPlane, "particles.dat");

    obs.Init();
    [[maybe_unused]] const LengthType length = obs.MaxStepLength(particle, track);
    [[maybe_unused]] const process::EProcessReturn ret =
        obs.DoContinuous(particle, track);

    SECTION("steplength") { REQUIRE(length == 12_m); }
  }
}
