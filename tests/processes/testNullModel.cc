/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/process/NullModel.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

using namespace corsika::units::si;
using namespace corsika::null_model;
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

  setup::Stack stack;
  setup::Stack::ParticleType particle = stack.AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::MomentumVector, geometry::Point, units::si::TimeType>{
          particles::Code::Electron, 100_GeV,
          corsika::MomentumVector(dummyCS, {0_GeV, 0_GeV, -1_GeV}),
          geometry::Point(dummyCS, {0_m, 0_m, 10_km}), 0_ns});
  SECTION("interface") {

    NullModel model(10_m);

    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoContinuous(particle, track);
    LengthType const length = model.MaxStepLength(particle, track);

    CHECK((length / 10_m) == Approx(1));
  }
}
