/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/stack/dummy/DummyStack.h>

using namespace corsika;
using namespace corsika::stack;

#include <catch2/catch.hpp>

#include <tuple>

TEST_CASE("DummyStack", "[stack]") {

  using TestStack = dummy::DummyStack;

  dummy::NoData noData;

  SECTION("write node") {

    TestStack s;
    s.AddParticle(std::tuple<dummy::NoData>{noData});
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("stack fill and cleanup") {

    TestStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) { s.AddParticle(std::tuple<dummy::NoData>{noData}); }

    REQUIRE(s.GetSize() == 99);
    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();
    REQUIRE(s.GetSize() == 0);
  }
}
