
/**
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

#include <corsika/process/stack_inspector/StackInspector.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using namespace corsika::units::si;
using namespace corsika::process::stack_inspector;
using namespace corsika;

TEST_CASE("StackInspector", "[processes]") {

  auto const& rootCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  geometry::Point const origin(rootCS, {0_m, 0_m, 0_m});
  geometry::Vector<corsika::units::si::SpeedType::dimension_type> v(
      rootCS, 0_m / second, 0_m / second, 1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  setup::Stack stack;
  auto particle = stack.AddParticle(
      particles::Code::Electron, 10_GeV,
      corsika::stack::super_stupid::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
      geometry::Point(rootCS, {0_m, 0_m, 10_km}), 0_ns);

  SECTION("interface") {

    StackInspector<setup::Stack> model(true);

    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoContinuous(particle, track, stack);
    [[maybe_unused]] const LengthType length = model.MaxStepLength(particle, track);
  }
}
