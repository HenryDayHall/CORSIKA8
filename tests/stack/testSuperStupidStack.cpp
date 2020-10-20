/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/stack/SuperStupidStack.hpp>

using namespace corsika;
using namespace corsika::units::si;

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::super_stupid;

using namespace std;

TEST_CASE("SuperStupidStack", "[stack]") {

  CoordinateSystem& dummyCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  SECTION("read+write") {

    SuperStupidStack s;
    s.AddParticle(
        std::tuple<corsika::Code, corsika::units::si::HEPEnergyType,
                   corsika::MomentumVector, corsika::Point, corsika::units::si::TimeType>{
            Code::Electron, 1.5_GeV,
            corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    // read
    CHECK(s.getEntries() == 1);
    CHECK(s.getSize() == 1);
    auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == Code::Electron);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
    // REQUIRE(pout.GetMomentum() == stack::MomentumVector(dummyCS, {1_GeV,
    // 1_GeV, 1_GeV})); REQUIRE(pout.GetPosition() == Point(dummyCS, {1 * meter, 1 *
    // meter, 1 * meter}));
    REQUIRE(pout.GetTime() == 100_s);
  }

  SECTION("write+delete") {

    SuperStupidStack s;
    for (int i = 0; i < 99; ++i)
      s.AddParticle(std::tuple<corsika::Code, corsika::units::si::HEPEnergyType,
                               corsika::MomentumVector, corsika::Point,
                               corsika::units::si::TimeType>{
          Code::Electron, 1.5_GeV,
          corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    CHECK(s.getSize() == 99);

    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();

    CHECK(s.getEntries() == 0);
    CHECK(s.getSize() == 1);
  }
}
