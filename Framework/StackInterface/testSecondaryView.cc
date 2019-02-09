
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/stack/SecondaryView.h>
#include <corsika/stack/Stack.h>

#include <testTestStack.h> // for testing: simple stack. This is a
// test-build, and inluce file is obtained from CMAKE_CURRENT_SOURCE_DIR

#include <boost/type_index.hpp>
#include <type_traits>
using boost::typeindex::type_id_with_cvr;

#include <iomanip>
#include <iostream>
#include <vector>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::stack;
using namespace std;

typedef Stack<TestStackData, TestParticleInterface> StackTest;
typedef StackTest::ParticleType Particle;

TEST_CASE("SecondaryStack", "[stack]") {

  // helper function for sum over stack data
  auto sum = [](const StackTest& stack) {
    double v = 0;
    for (const auto& p : stack) v += p.GetData();
    return v;
  };

  SECTION("secondary view") {
    StackTest s;
    REQUIRE(s.GetSize() == 0);
    s.AddParticle(std::tuple{9.9});
    s.AddParticle(std::tuple{8.8});
    const double sumS = 9.9 + 8.8;

    auto particle = s.GetNextParticle();

    typedef SecondaryView<TestStackData, TestParticleInterface> StackTestView;
    StackTestView v(particle);
    REQUIRE(v.GetSize() == 0);

    {
      auto proj = v.GetProjectile();
      REQUIRE(proj.GetData() == particle.GetData());
    }

    v.AddSecondary(std::tuple{4.4});
    v.AddSecondary(std::tuple{4.5});
    v.AddSecondary(std::tuple{4.6});

    REQUIRE(v.GetSize() == 3);
    REQUIRE(s.GetSize() == 5);
    REQUIRE(!v.IsEmpty());

    auto sumView = [](const StackTestView& stack) {
      double v = 0;
      for (const auto& p : stack) { v += p.GetData(); }
      return v;
    };

    REQUIRE(sum(s) == sumS + 4.4 + 4.5 + 4.6);
    REQUIRE(sumView(v) == 4.4 + 4.5 + 4.6);

    v.DeleteLast();
    REQUIRE(v.GetSize() == 2);
    REQUIRE(s.GetSize() == 4);

    REQUIRE(sum(s) == sumS + 4.4 + 4.5);
    REQUIRE(sumView(v) == 4.4 + 4.5);

    auto pDel = v.GetNextParticle();
    v.Delete(pDel);
    REQUIRE(v.GetSize() == 1);
    REQUIRE(s.GetSize() == 3);

    REQUIRE(sum(s) == sumS + 4.4 + 4.5 - pDel.GetData());
    REQUIRE(sumView(v) == 4.4 + 4.5 - pDel.GetData());

    v.Delete(v.GetNextParticle());
    REQUIRE(sum(s) == sumS);
    REQUIRE(sumView(v) == 0);
    REQUIRE(v.IsEmpty());

    {
      auto proj = v.GetProjectile();
      REQUIRE(proj.GetData() == particle.GetData());
    }
  }
}
