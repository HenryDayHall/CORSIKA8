
/**
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

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::stack::super_stupid;

#include <iostream>
using namespace std;

TEST_CASE("SuperStupidStack", "[stack]") {

  SECTION("read+write") {

    SuperStupidStack s;
    auto p = s.NewParticle();
    p.SetPID(particles::Code::Electron);
    p.SetEnergy(1.5_GeV);
    geometry::CoordinateSystem& dummyCS =
        geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
    p.SetMomentum(MomentumVector(
        dummyCS, {1 * newton_second, 1 * newton_second, 1 * newton_second}));
    p.SetPosition(Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}));
    p.SetTime(100_s);

    // read
    REQUIRE(s.GetSize() == 1);
    auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == particles::Code::Electron);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
#warning Fix the next two lines:
    // REQUIRE(pout.GetMomentum() == MomentumVector(dummyCS, {1 * joule, 1 * joule, 1 *
    // joule})); REQUIRE(pout.GetPosition() == Point(dummyCS, {1 * meter, 1 * meter, 1 *
    // meter}));
    REQUIRE(pout.GetTime() == 100_s);
  }

  SECTION("write+delete") {

    SuperStupidStack s;
    for (int i = 0; i < 99; ++i) s.NewParticle();

    REQUIRE(s.GetSize() == 99);

    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();

    REQUIRE(s.GetSize() == 0);
  }
}
