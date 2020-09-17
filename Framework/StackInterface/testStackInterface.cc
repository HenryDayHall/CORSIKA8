/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define protected public // to also test the internal state of objects

#include <corsika/stack/Stack.h>

#include <testTestStack.h> // simple test-stack for testing. This is
                           // for testing only: include from
                           // CMAKE_CURRENT_SOURCE_DIR

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
    CHECK(s.getSize() == 1);
  }

  SECTION("construct") {

    // construct a valid, empty Stack object
    StackTest s;
  }

  SECTION("write and read") {

    StackTest s;
    s.AddParticle(std::tuple{9.9});
    const double v = sum(s);
    CHECK(v == 9.9);
  }

  SECTION("delete from stack") {

    StackTest s;
    CHECK(s.getSize() == 0);
    StackTest::StackIterator p =
        s.AddParticle(std::tuple{0.}); // valid way to access particle data
    p.SetData(9.9);
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 1);
    s.Delete(p);
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
  }

  SECTION("delete particle") {

    StackTest s;
    CHECK(s.getSize() == 0);
    s.AddParticle(std::tuple{8.9});
    s.AddParticle(std::tuple{7.9});
    auto p = s.AddParticle(
        std::tuple{9.9}); // also valid way to access particle data, identical to above

    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 3);
    CHECK(!s.IsEmpty());

    p.Delete(); // mark for deletion: size=3, entries=2
    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 2);
    CHECK(!s.IsEmpty());

    s.last().Delete(); // mark for deletion: size=3, entries=1
    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 1);
    CHECK(!s.IsEmpty());

    /*
       GetNextParticle will find two entries marked as "deleted" and
       will purge this from the end of the stack: size = 1
    */
    s.GetNextParticle().Delete(); // mark for deletion: size=3, entries=0
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
    CHECK(s.IsEmpty());
  }

  SECTION("create secondaries") {

    StackTest s;
    CHECK(s.getSize() == 0);
    auto iter = s.AddParticle(std::tuple{9.9});
    StackTest::ParticleInterfaceType& p =
        *iter; // also this is valid to access particle data
    CHECK(s.getSize() == 1);
    p.AddSecondary(std::tuple{4.4});
    CHECK(s.getSize() == 2);
    /*p.AddSecondary(3.3, 2.2);
    CHECK(s.getSize() == 3);
    double v = 0;
    for (auto& p : s) { v += p.GetData(); }
    CHECK(v == 9.9 + 4.4 + 3.3 + 2.2);*/
  }

  SECTION("get next particle") {
    StackTest s;
    CHECK(s.getSize() == 0);
    CHECK(s.getEntries() == 0);
    CHECK(s.IsEmpty());

    s.AddParticle(std::tuple{9.9});
    s.AddParticle(std::tuple{8.8});
    CHECK(s.getSize() == 2);
    CHECK(s.getEntries() == 2);
    CHECK(!s.IsEmpty());

    auto particle = s.GetNextParticle(); // first particle
    CHECK(particle.GetData() == 8.8);

    particle.Delete(); // only marks (last) particle as deleted
    CHECK(s.getSize() == 2);
    CHECK(s.getEntries() == 1);
    CHECK(!s.IsEmpty());

    /*
      This following call to GetNextParticle will realize that the
      current last particle on the stack was marked "deleted" and will
      purge it: stack size is reduced by one.
     */
    auto particle2 = s.GetNextParticle(); // first particle
    CHECK(particle2.GetData() == 9.9);
    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 1);
    CHECK(!s.IsEmpty());

    particle2.Delete(); // also mark this particle as deleted

    CHECK(s.getSize() == 1);
    CHECK(s.getEntries() == 0);
    CHECK(s.IsEmpty());
  }
}
