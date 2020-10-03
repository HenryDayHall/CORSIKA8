/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define protected public // to also test the internal state of objects

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika::geometry;
using namespace corsika::units::si;

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::stack::super_stupid;

using namespace std;

TEST_CASE("SuperStupidStack", "[stack]") {

  geometry::CoordinateSystem& dummyCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  SECTION("read+write") {

    SuperStupidStack s;
    s.AddParticle(std::make_tuple{
        particles::Code::Electron, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    // read
    CHECK(s.getEntries() == 1);
    CHECK(s.getSize() == 1);
    auto pout = s.GetNextParticle();
    CHECK(pout.GetPID() == particles::Code::Electron);
    CHECK(pout.GetEnergy() == 1.5_GeV);
    CHECK(pout.GetTime() == 100_s);
  }

  SECTION("write+delete") {

    SuperStupidStack s;
    for (int i = 0; i < 99; ++i)
      s.AddParticle(
          std::make_tuple{
              particles::Code::Electron, 1.5_GeV,
              corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
              Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    CHECK(s.getSize() == 99);

    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();

    CHECK(s.getEntries() == 0);
    CHECK(s.getSize() == 1);
  }
}
