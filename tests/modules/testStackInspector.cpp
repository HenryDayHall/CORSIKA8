/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/modules/StackInspector.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <../framework/testCascade.hpp> //! \todo fix this

using namespace corsika;

TEST_CASE("StackInspector", "modules") {

  logging::set_level(logging::level::info);

  auto const& rootCS = get_root_CoordinateSystem();
  Point const origin(rootCS, {0_m, 0_m, 0_m});
  VelocityVector v(rootCS, 0_m / second, 0_m / second, 1_m / second);
  Line line(origin, v);
  StraightTrajectory track(line, 10_s);

  TestCascadeStack stack;
  stack.clear();
  HEPEnergyType E0 = 100_GeV;
  stack.addParticle(std::make_tuple(Code::Electron,
                                    MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
                                    Point(rootCS, {0_m, 0_m, 10_km}), 0_ns));
  stack.addParticle(std::make_tuple(Code::Nucleus,
                                    MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
                                    Point(rootCS, {0_m, 0_m, 10_km}), 0_ns, 16, 8));

  SECTION("interface") {

    StackInspector<TestCascadeStack> model(1, true, E0);
    model.doStack(stack);
    // there are no actions, nothing to check...
  }
}
