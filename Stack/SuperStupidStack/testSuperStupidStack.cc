/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika::geometry;
using namespace corsika::units::si;

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::stack::super_stupid;

#include <iostream>
using namespace std;

TEST_CASE("SuperStupidStack", "[stack]") {

  geometry::CoordinateSystem& dummyCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  SECTION("read+write") {

    SuperStupidStack s;
    s.AddParticle(std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                             corsika::stack::MomentumVector, corsika::geometry::Point,
                             corsika::units::si::TimeType>{
        particles::Code::Electron, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    // read
    REQUIRE(s.GetSize() == 1);
    auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == particles::Code::Electron);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
    // REQUIRE(pout.GetMomentum() == stack::MomentumVector(dummyCS, {1_GeV,
    // 1_GeV, 1_GeV})); REQUIRE(pout.GetPosition() == Point(dummyCS, {1 * meter, 1 *
    // meter, 1 * meter}));
    REQUIRE(pout.GetTime() == 100_s);
  }

  SECTION("write+delete") {

    SuperStupidStack s;
    for (int i = 0; i < 99; ++i)
      s.AddParticle(
          std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                     corsika::stack::MomentumVector, corsika::geometry::Point,
                     corsika::units::si::TimeType>{
              particles::Code::Electron, 1.5_GeV,
              corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
              Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    REQUIRE(s.GetSize() == 99);

    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();

    REQUIRE(s.GetSize() == 0);
  }
}
