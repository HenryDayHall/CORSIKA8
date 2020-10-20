/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/process/stack_inspector/StackInspector.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/cascade/testCascade.h>

using namespace corsika::units::si;
using namespace corsika::process::stack_inspector;
using namespace corsika;
using namespace corsika::geometry;

TEST_CASE("StackInspector", "[processes]") {

  auto const& rootCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  geometry::Point const origin(rootCS, {0_m, 0_m, 0_m});
  geometry::Vector<units::si::SpeedType::dimension_type> v(rootCS, 0_m / second,
                                                           0_m / second, 1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  TestCascadeStack stack;
  stack.Clear();
  HEPEnergyType E0 = 100_GeV;
  stack.AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
          particles::Code::Electron, E0,
          corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
          Point(rootCS, {0_m, 0_m, 10_km}), 0_ns});

  SECTION("interface") {

    StackInspector<TestCascadeStack> model(1, true, E0);

    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret = model.DoStack(stack);
  }
}
