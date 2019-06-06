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
  setup::Stack::ParticleType particle = stack.AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
          particles::Code::Electron, 100_GeV,
          corsika::stack::MomentumVector(dummyCS, {0_GeV, 0_GeV, -1_GeV}),
          geometry::Point(dummyCS, {0_m, 0_m, 10_km}), 0_ns});
  SECTION("interface") {

    NullModel model(10_m);

    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoContinuous(particle, track);
    LengthType const length = model.MaxStepLength(particle, track);

    CHECK((length / 10_m) == Approx(1));
  }
}
