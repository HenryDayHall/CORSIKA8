/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define protected public // to also test the internal state of objects

#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/stack/SuperStupidStack.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace std;

TEST_CASE("SuperStupidStack", "[stack]") {

  const CoordinateSystemPtr& dummyCS = get_root_CoordinateSystem();

  SECTION("read+write") {

    simple_stack::SuperStupidStack s;
    s.addParticle(
        std::make_tuple(Code::Electron, 1.5_GeV,
                        MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

    // read
    CHECK(s.getEntries() == 1);
    CHECK(s.getSize() == 1);
    auto pout = s.getNextParticle();
    CHECK(pout.getPID() == Code::Electron);
    CHECK(pout.getEnergy() == 1.5_GeV);
    CHECK(pout.getTime() == 100_s);
  }

  SECTION("write+delete") {

    simple_stack::SuperStupidStack s;
    for (int i = 0; i < 99; ++i)
      s.addParticle(
          std::make_tuple(Code::Electron, 1.5_GeV,
                          MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

    CHECK(s.getSize() == 99);

    for (int i = 0; i < 99; ++i) s.getNextParticle().erase();

    CHECK(s.getEntries() == 0);
    CHECK(s.getSize() == 1);
  }
}
