/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

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

  auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  auto [stack, view] =
      setup::testing::setupStack(particles::Code::Electron, 0, 0, 100_GeV, nodePtr, cs);
  [[maybe_unused]] const auto& dummyView = view;
  auto particle = stack->first();

  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  geometry::Vector<units::si::SpeedType::dimension_type> v(cs, 0_m / second, 0_m / second,
                                                           1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  SECTION("interface") {

    NullModel model(10_m);

    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoContinuous(particle, track);
    LengthType const length = model.MaxStepLength(particle, track);

    CHECK((length / 10_m) == Approx(1));
  }
}
