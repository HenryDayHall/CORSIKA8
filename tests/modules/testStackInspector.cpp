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

#include <corsika/modules/stack_inspector/StackInspector.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <../framework/testCascade.hpp>

using namespace corsika::units::si;
using namespace corsika::stack_inspector;
using namespace corsika;

TEST_CASE("StackInspector", "[processes]") {

  auto const& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  Point const origin(rootCS, {0_m, 0_m, 0_m});
  Vector<units::si::SpeedType::dimension_type> v(rootCS, 0_m / second,
                                                           0_m / second, 1_m / second);
  Line line(origin, v);
  Trajectory<Line> track(line, 10_s);

  TestCascadeStack stack;
  stack.Clear();
  HEPEnergyType E0 = 100_GeV;
  stack.AddParticle(
      std::tuple<corsika::Code, units::si::HEPEnergyType,
                 corsika::MomentumVector, Point, units::si::TimeType>{
          Code::Electron, E0,
          corsika::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
          Point(rootCS, {0_m, 0_m, 10_km}), 0_ns});

  SECTION("interface") {

    StackInspector<TestCascadeStack> model(1, true, E0);

    model.Init();
    [[maybe_unused]] const corsika::EProcessReturn ret = model.DoStack(stack);
  }
}
