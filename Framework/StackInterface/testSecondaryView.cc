/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define protected public // to also test the internal state of objects

#include <corsika/stack/SecondaryView.h>
#include <corsika/stack/Stack.h>

#include <testTestStack.h> // for testing: simple stack. This is a
// test-build, and inluce file is obtained from CMAKE_CURRENT_SOURCE_DIR

#include <iomanip>
#include <iostream>
#include <vector>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::stack;
using namespace std;

typedef Stack<TestStackData, TestParticleInterface> StackTest;

/*
  See Issue 161

  unfortunately clang does not support this in the same way (yet) as
  gcc, so we have to distinguish here. If clang cataches up, we could
  remove the clang branch here and also in corsika::Cascade. The gcc
  code is much more generic and universal.
 */
#if defined(__clang__)
using StackTestView = SecondaryView<TestStackData, TestParticleInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
using StackTestView = MakeView<StackTest>::type;
#endif

using Particle = typename StackTest::ParticleType;

TEST_CASE("SecondaryStack", "[stack]") {

  // helper function for sum over stack data
  auto sum = [](const StackTest& stack) {
    double v = 0;
    for (const auto& p : stack) { v += p.GetData(); }
    return v;
  };

  auto sumView = [](const StackTestView& stack) {
    double value = 0;
    for (const auto& p : stack) { value += p.GetData(); }
    return value;
  };

  SECTION("secondary view") {
    StackTest s;
    CHECK(s.getSize() == 0);
    CHECK(s.IsEmpty());

    s.AddParticle(std::tuple{9.9});
    s.AddParticle(std::tuple{8.8});
    const double sumS = 9.9 + 8.8; // helper, see below
    CHECK(s.getSize() == 2);
    CHECK(s.getEntries() == 2);
    CHECK(!s.IsEmpty());

    auto particle = s.GetNextParticle();

    StackTestView view(particle);
    CHECK(view.getSize() == 0);
    CHECK(view.getEntries() == 0);
    CHECK(view.IsEmpty());

    {
      auto proj = view.GetProjectile();
      CHECK(proj.GetData() == particle.GetData());
      proj.AddSecondary(std::tuple{4.4});
    }
    CHECK(view.getSize() == 1);
    CHECK(view.getEntries() == 1);
    CHECK(!view.IsEmpty());
    CHECK(s.getSize() == 3);
    CHECK(s.getEntries() == 3);
    CHECK(!s.IsEmpty());

    view.AddSecondary(std::tuple{4.5});
    view.AddSecondary(std::tuple{4.6});
    CHECK(view.getSize() == 3);
    CHECK(view.getEntries() == 3);
    CHECK(!view.IsEmpty());
    CHECK(s.getSize() == 5);
    CHECK(s.getEntries() == 5);
    CHECK(!s.IsEmpty());

    CHECK(sum(s) == sumS + 4.4 + 4.5 + 4.6);
    CHECK(sumView(view) == 4.4 + 4.5 + 4.6);

    view.last().Delete();
    CHECK(view.getSize() == 3);
    CHECK(view.getEntries() == 2);
    CHECK(s.getSize() == 5);
    CHECK(s.getEntries() == 4);

    CHECK(sum(s) == sumS + 4.4 + 4.5);
    CHECK(sumView(view) == 4.4 + 4.5);

    auto pDel = view.GetNextParticle();
    view.Delete(pDel);
    CHECK(view.getSize() == 2);
    CHECK(s.getSize() == 4);

    CHECK(sum(s) == sumS + 4.4 + 4.5 - pDel.GetData());
    CHECK(sumView(view) == 4.4 + 4.5 - pDel.GetData());

    view.Delete(view.GetNextParticle());
    CHECK(sum(s) == sumS);
    CHECK(sumView(view) == 0);
    CHECK(view.IsEmpty());

    {
      auto proj = view.GetProjectile();
      CHECK(proj.GetData() == particle.GetData());
    }
  }

  SECTION("secondary view, construct from ParticleType") {
    StackTest s;
    CHECK(s.getSize() == 0);
    s.AddParticle(std::tuple{9.9});
    s.AddParticle(std::tuple{8.8});

    auto iterator = s.GetNextParticle();
    typename StackTest::ParticleType& particle = iterator; // as in corsika::Cascade

    StackTestView view(particle);
    CHECK(view.getSize() == 0);

    view.AddSecondary(std::tuple{4.4});

    CHECK(view.getSize() == 1);
  }

  SECTION("deletion") {
    StackTest stack;
    stack.AddParticle(std::tuple{-99.});
    stack.AddParticle(std::tuple{0.});

    {
      auto particle = stack.GetNextParticle();
      StackTestView view(particle);

      auto proj = view.GetProjectile();
      proj.AddSecondary(std::tuple{-2.});
      proj.AddSecondary(std::tuple{-1.});
      proj.AddSecondary(std::tuple{1.});
      proj.AddSecondary(std::tuple{2.});

      CHECK(stack.getSize() == 6); // -99, 0, -2, -1, 1, 2
      CHECK(view.getSize() == 4);  // -2, -1, 1, 2

      // now delete all negative entries, i.e. -1 and -2
      auto p = view.begin();
      while (p != view.end()) {
        auto data = p.GetData();
        if (data < 0) { p.Delete(); }
        ++p;
      }
      CHECK(stack.getSize() == 6);
      CHECK(stack.getEntries() == 4); // -99, 0, 2, 1 (order changes during deletion)
      CHECK(view.getSize() == 4);
      CHECK(view.getEntries() == 2); // 2, 1
    }

    // repeat

    {
      auto particle = stack.GetNextParticle();
      StackTestView view(particle);

      // put -2,...,+2 on stack
      auto proj = view.GetProjectile();
      proj.AddSecondary(std::tuple{-2.});
      proj.AddSecondary(std::tuple{-1.});
      proj.AddSecondary(std::tuple{1.});
      proj.AddSecondary(std::tuple{2.});
      // stack should contain -99, 0, 2, 1, [-2, -1, 1, 2]

      auto p = view.begin();
      while (p != view.end()) {
        auto data = p.GetData();
        if (data < 0) { p.Delete(); }
        ++p;
      }

      // stack should contain -99, 0, 2, 1, [2, 1]
      // view should contain 1, 2

      CHECK(stack.getEntries() == 6);
      CHECK(stack.getSize() == 10);
    }
  }
}
