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
    StackTest stack;
    stack.Clear();
    CHECK(stack.getSize() == 0);
    CHECK(stack.IsEmpty());                          // stack empty here
    auto pTest0 = stack.AddParticle(std::tuple{0.}); // [0]
    CHECK(stack.getSize() == 1);
    CHECK(!stack.IsEmpty());
    auto pTest1 = stack.AddParticle(std::tuple{1.}); // [0,1]
    CHECK(stack.getSize() == 2);
    CHECK(pTest1.GetData() == 1.);
    auto pTestAt = stack.at(1); // -> 1
    CHECK(pTestAt == pTest1);
    auto pTestFirst = stack.first(); // -> 0
    CHECK(pTestFirst == pTest0);
  }

  SECTION("construct") {

    // construct a valid, empty Stack object
    StackTest s;
  }

  SECTION("write and read") {

    StackTest stack;
    stack.AddParticle(std::tuple{9.9});
    const double v = sum(stack);
    CHECK(v == 9.9);
  }

  SECTION("delete from stack") {

    StackTest stack;
    CHECK(stack.getSize() == 0);
    StackTest::StackIterator p =
        stack.AddParticle(std::tuple{0.}); // valid way to access particle data
    p.SetData(9.9);
    CHECK(stack.getSize() == 1);
    CHECK(stack.getEntries() == 1);
    stack.Delete(p);
    CHECK(stack.getSize() == 1);
    CHECK(stack.getEntries() == 0);
  }

  SECTION("delete particle") {

    StackTest stack;
    CHECK(stack.getSize() == 0);
    stack.AddParticle(std::tuple{8.9});
    stack.AddParticle(std::tuple{7.9});
    auto p = stack.AddParticle(
        std::tuple{9.9}); // also valid way to access particle data, identical to above

    CHECK(stack.getSize() == 3);
    CHECK(stack.getEntries() == 3);
    CHECK(!stack.IsEmpty());

    p.Delete(); // mark for deletion: size=3, entries=2
    CHECK(stack.getSize() == 3);
    CHECK(stack.getEntries() == 2);
    CHECK(!stack.IsEmpty());

    stack.last().Delete(); // mark for deletion: size=3, entries=1
    CHECK(stack.getSize() == 3);
    CHECK(stack.getEntries() == 1);
    CHECK(!stack.IsEmpty());

    /*
       GetNextParticle will find two entries marked as "deleted" and
       will purge this from the end of the stack: size = 1
    */
    stack.GetNextParticle().Delete(); // mark for deletion: size=3, entries=0
    CHECK(stack.getSize() == 1);
    CHECK(stack.getEntries() == 0);
    CHECK(stack.IsEmpty());
  }

  SECTION("create secondaries") {

    StackTest stack;
    CHECK(stack.getSize() == 0);
    auto iter = stack.AddParticle(std::tuple{9.9});
    StackTest::ParticleInterfaceType& p =
        *iter; // also this is valid to access particle data
    CHECK(stack.getSize() == 1);
    p.AddSecondary(std::tuple{4.4});
    CHECK(stack.getSize() == 2);
  }

  SECTION("get next particle") {
    StackTest stack;
    CHECK(stack.getSize() == 0);
    CHECK(stack.getEntries() == 0);
    CHECK(stack.IsEmpty());

    stack.AddParticle(std::tuple{9.9});
    stack.AddParticle(std::tuple{8.8});
    CHECK(stack.getSize() == 2);
    CHECK(stack.getEntries() == 2);
    CHECK(!stack.IsEmpty());

    auto particle = stack.GetNextParticle(); // first particle
    CHECK(particle.GetData() == 8.8);

    particle.Delete(); // only marks (last) particle as deleted
    CHECK(stack.getSize() == 2);
    CHECK(stack.getEntries() == 1);
    CHECK(!stack.IsEmpty());

    /*
      This following call to GetNextParticle will realize that the
      current last particle on the stack was marked "deleted" and will
      purge it: stack size is reduced by one.
     */
    auto particle2 = stack.GetNextParticle(); // first particle
    CHECK(particle2.GetData() == 9.9);
    CHECK(stack.getSize() == 1);
    CHECK(stack.getEntries() == 1);
    CHECK(!stack.IsEmpty());

    particle2.Delete(); // also mark this particle as deleted

    CHECK(stack.getSize() == 1);
    CHECK(stack.getEntries() == 0);
    CHECK(stack.IsEmpty());
  }

  SECTION("swap particle") {
    StackTest stack;
    CHECK(stack.getSize() == 0);
    CHECK(stack.getEntries() == 0);
    CHECK(stack.IsEmpty());

    stack.AddParticle(std::tuple{9.888});
    stack.AddParticle(std::tuple{8.999});
    CHECK(stack.getSize() == 2);
    CHECK(stack.getEntries() == 2);
    CHECK(!stack.IsEmpty());

    auto p1 = stack.begin();
    auto p2 = p1 + 1;

    CHECK(p1.GetData() == 9.888);
    CHECK(p2.GetData() == 8.999);

    stack.Swap(p1, p2);

    CHECK(p1.GetData() == 8.999);
    CHECK(p2.GetData() == 9.888);
  }

  SECTION("copy particle") {
    StackTest stack;
    CHECK(stack.getSize() == 0);
    CHECK(stack.getEntries() == 0);
    CHECK(stack.IsEmpty());

    stack.AddParticle(std::tuple{9.888});
    stack.AddParticle(std::tuple{8.999});
    CHECK(stack.getSize() == 2);
    CHECK(stack.getEntries() == 2);
    CHECK(!stack.IsEmpty());

    auto p1 = stack.begin();
    auto p2 = p1 + 1;

    CHECK(p1.GetData() == 9.888);
    CHECK(p2.GetData() == 8.999);

    stack.Copy(p1, p2);

    CHECK(p1.GetData() == 9.888);
    CHECK(p2.GetData() == 9.888);
  }
}
