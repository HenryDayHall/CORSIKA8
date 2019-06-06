/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/stack/Stack.h>

#include <testTestStack.h> // simple test-stack for testing. This is
                           // for testing only: include from
                           // CMAKE_CURRENT_SOURCE_DIR

#include <boost/type_index.hpp>
#include <type_traits>
using boost::typeindex::type_id_with_cvr;

#include <iomanip>
#include <iostream>
#include <tuple>
#include <vector>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::stack;
using namespace std;

typedef Stack<TestStackData, TestParticleInterface> StackTest;

TEST_CASE("Stack", "[Stack]") {

  // helper function for sum over stack data
  auto sum = [](const StackTest& stack) {
    double v = 0;
    for (const auto& p : stack) v += p.GetData();
    return v;
  };

  SECTION("StackInterface") {

    // construct a valid Stack object
    StackTest s;
    s.Clear();
    s.AddParticle(std::tuple{0.});
    s.Copy(s.cbegin(), s.begin());
    s.Swap(s.begin(), s.begin());
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("construct") {

    // construct a valid, empty Stack object
    StackTest s;
  }

  SECTION("write and read") {

    StackTest s;
    s.AddParticle(std::tuple{9.9});
    const double v = sum(s);
    REQUIRE(v == 9.9);
  }

  SECTION("delete from stack") {

    StackTest s;
    REQUIRE(s.GetSize() == 0);
    StackTest::StackIterator p =
        s.AddParticle(std::tuple{0.}); // valid way to access particle data
    p.SetData(9.9);
    REQUIRE(s.GetSize() == 1);
    s.Delete(p);
    REQUIRE(s.GetSize() == 0);
  }

  SECTION("delete particle") {

    StackTest s;
    REQUIRE(s.GetSize() == 0);
    auto p = s.AddParticle(
        std::tuple{9.9}); // also valid way to access particle data, identical to above
    REQUIRE(s.GetSize() == 1);
    p.Delete();
    REQUIRE(s.GetSize() == 0);
  }

  SECTION("create secondaries") {

    StackTest s;
    REQUIRE(s.GetSize() == 0);
    auto iter = s.AddParticle(std::tuple{9.9});
    StackTest::ParticleInterfaceType& p =
        *iter; // also this is valid to access particle data
    REQUIRE(s.GetSize() == 1);
    p.AddSecondary(std::tuple{4.4});
    REQUIRE(s.GetSize() == 2);
    /*p.AddSecondary(3.3, 2.2);
    REQUIRE(s.GetSize() == 3);
    double v = 0;
    for (auto& p : s) { v += p.GetData(); }
    REQUIRE(v == 9.9 + 4.4 + 3.3 + 2.2);*/
  }

  SECTION("get next particle") {
    StackTest s;
    REQUIRE(s.GetSize() == 0);
    s.AddParticle(std::tuple{9.9});
    s.AddParticle(std::tuple{8.8});
    auto particle = s.GetNextParticle(); // first particle
    REQUIRE(particle.GetData() == 8.8);

    particle.Delete();
    auto particle2 = s.GetNextParticle(); // first particle
    REQUIRE(particle2.GetData() == 9.9);
    particle2.Delete();

    REQUIRE(s.GetSize() == 0);
  }
}
