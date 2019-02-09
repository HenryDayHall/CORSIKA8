
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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

#include <corsika/process/null_model/NullModel.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using namespace corsika::units::si;
using namespace corsika::process::null_model;
using namespace corsika;

TEST_CASE("NullModel", "[processes]") {

  auto const& dummyCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  geometry::Point const origin(dummyCS, {0_m, 0_m, 0_m});
  geometry::Vector<units::si::SpeedType::dimension_type> v(dummyCS, 0_m / second,
                                                           0_m / second, 1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  setup::Stack stack;
  auto particle = stack.AddParticle(
      std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, corsika::geometry::Point,
                 corsika::units::si::TimeType>{
          particles::Code::Electron, 1.5_GeV,
          stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
          geometry::Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

  SECTION("interface") {

    NullModel model(10_m);

    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoContinuous(particle, track, stack);
    LengthType const length = model.MaxStepLength(particle, track);

    CHECK((length / 10_m) == Approx(1));
  }
}
