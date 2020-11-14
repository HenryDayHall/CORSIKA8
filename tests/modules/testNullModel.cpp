/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/modules/NullModel.hpp>

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

  auto const& dummyCS =
      corsika::RootCoordinateSystem::getInstance().GetRootCoordinateSystem();
  corsika::Point const origin(dummyCS, {0_m, 0_m, 0_m});
  corsika::Vector<SpeedType::dimension_type> v(dummyCS, 0_m / second, 0_m / second,
                                               1_m / second);
  corsika::Line line(origin, v);
  corsika::Trajectory<corsika::Line> track(line, 10_s);

  setup::Stack stack;
  setup::Stack::ParticleType particle = stack.AddParticle(
      std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector, corsika::Point,
                 units::si::TimeType>{
          Code::Electron, 100_GeV,
          corsika::MomentumVector(dummyCS, {0_GeV, 0_GeV, -1_GeV}),
          corsika::Point(dummyCS, {0_m, 0_m, 10_km}), 0_ns});

  SECTION("interface") {

    NullModel model(10_m);

    model.Init();
    [[maybe_unused]] const EProcessReturn ret = model.DoContinuous(particle, track);
    LengthType const length = model.MaxStepLength(particle, track);

    CHECK((length / 10_m) == Approx(1));
  }
}
