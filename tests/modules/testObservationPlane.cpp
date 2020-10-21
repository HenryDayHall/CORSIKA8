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

using namespace corsika::observation_plane;
using namespace corsika;

TEST_CASE("ContinuousProcess interface", "[proccesses][observation_plane]") {

  auto const& rootCS = RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  /*
    Test with downward going 1_GeV neutrino, starting at 0,1_m,10m

    ObservationPlane has origin at 0,0,0
   */

  Point const start(rootCS, {0_m, 1_m, 10_m});
  Vector<SpeedType::dimension_type> vec(rootCS, 0_m / second, 0_m / second,
                                        -constants::c);
  Line line(start, vec);
  Trajectory<Line> track(line, 12_m / constants::c);

  // setup particle stack, and add primary particle
  setup::Stack stack;
  stack.Clear();
  {
    auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
      return sqrt((Elab - m) * (Elab + m));
    };
    stack.AddParticle(
        std::tuple<Code, HEPEnergyType, corsika::MomentumVector, Point, TimeType>{
            Code::NuMu, 1_GeV,
            corsika::MomentumVector(rootCS,
                                    {0_GeV, 0_GeV, -elab2plab(1_GeV, NuMu::GetMass())}),
            Point(rootCS, {1_m, 1_m, 10_m}), 0_ns});
  }
  auto particle = stack.GetNextParticle();

  SECTION("horizontal plane") {

    Plane const obsPlane(Point(rootCS, {0_m, 0_m, 0_m}),
                         Vector<dimensionless_d>(rootCS, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, "particles.dat", true);

    obs.Init();
    const LengthType length = obs.MaxStepLength(particle, track);
    const EProcessReturn ret = obs.DoContinuous(particle, track);

    REQUIRE(length / 10_m == Approx(1).margin(1e-4));
    REQUIRE(ret == EProcessReturn::eParticleAbsorbed);

    /*
    SECTION("horizontal plane") {
      REQUIRE(true); // todo: we have to check content of output file...

    }
    */
  }

  SECTION("inclined plane") {}

  SECTION("transparent plane") {
    Plane const obsPlane(Point(rootCS, {0_m, 0_m, 0_m}),
                         Vector<dimensionless_d>(rootCS, {0., 0., 1.}));
    ObservationPlane obs(obsPlane, "particles.dat", false);

    obs.Init();
    const LengthType length = obs.MaxStepLength(particle, track);
    const EProcessReturn ret = obs.DoContinuous(particle, track);

    REQUIRE(length / 10_m == Approx(1).margin(1e-4));
    REQUIRE(ret == EProcessReturn::eOk);
  }
}
