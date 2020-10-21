/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define protected public // to also test the internal state of objects

#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/framework/stack/Stack.hpp>

#include <testTestStack.h> // for testing: simple stack. This is a
// test-build, and inluce file is obtained from CMAKE_CURRENT_SOURCE_DIR

#include <iomanip>
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

  logging::SetLevel(logging::level::debug);

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
    StackTest stack;
    CHECK(stack.getSize() == 0);
    CHECK(stack.IsEmpty());

    stack.AddParticle(std::tuple{9.9});
    stack.AddParticle(std::tuple{8.8});
    const double sumS = 9.9 + 8.8; // helper, see below
    CHECK(stack.getSize() == 2);
    CHECK(stack.getEntries() == 2);
    CHECK(!stack.IsEmpty());

    auto particle = stack.GetNextParticle();

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
    CHECK(stack.getSize() == 3);
    CHECK(stack.getEntries() == 3);
    CHECK(!stack.IsEmpty());

    view.AddSecondary(std::tuple{4.5});
    view.AddSecondary(std::tuple{4.6});
    CHECK(view.getSize() == 3);
    CHECK(view.getEntries() == 3);
    CHECK(!view.IsEmpty());
    CHECK(stack.getSize() == 5);
    CHECK(stack.getEntries() == 5);
    CHECK(!stack.IsEmpty());

    CHECK(sum(stack) == sumS + 4.4 + 4.5 + 4.6);
    CHECK(sumView(view) == 4.4 + 4.5 + 4.6);

    view.last().Delete();
    CHECK(view.getSize() == 3);
    CHECK(view.getEntries() == 2);
    CHECK(stack.getSize() == 5);
    CHECK(stack.getEntries() == 4);

    CHECK(sum(stack) == sumS + 4.4 + 4.5);
    CHECK(sumView(view) == 4.4 + 4.5);

    auto pDel = view.GetNextParticle();
    view.Delete(pDel);
    CHECK(view.getSize() == 2);
    CHECK(stack.getSize() == 4);

    CHECK(sum(stack) == sumS + 4.4 + 4.5 - pDel.GetData());
    CHECK(sumView(view) == 4.4 + 4.5 - pDel.GetData());

    view.Delete(view.GetNextParticle());
    CHECK(sum(stack) == sumS);
    CHECK(sumView(view) == 0);
    CHECK(view.IsEmpty());

    {
      auto proj = view.GetProjectile();
      CHECK(proj.GetData() == particle.GetData());
      CHECK(particle == view.parent());
    }

    // at() and first()
    auto pTestAt = view.at(0);
    auto pTestFirst = view.first();
    CHECK(pTestFirst == pTestAt);
  }

  SECTION("secondary view, construct from ParticleType") {
    StackTest stack;
    CHECK(stack.getSize() == 0);
    stack.AddParticle(std::tuple{9.9});
    stack.AddParticle(std::tuple{8.8});

    auto iterator = stack.GetNextParticle();
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

  SECTION("swap particle") {
    StackTest stack;
    stack.AddParticle(std::tuple{-99.});

    StackTestView view(stack.first());
    view.AddSecondary(std::tuple{-2.});
    view.AddSecondary(std::tuple{-1.});
    view.AddSecondary(std::tuple{1.});

    auto p1 = view.begin();
    auto p2 = p1 + 1;

    CHECK(p1.GetData() == -2.);
    CHECK(p2.GetData() == -1.);

    view.Swap(p1, p2);

    CHECK(p1.GetData() == -1);
    CHECK(p2.GetData() == -2);
  }

  SECTION("copy particle") {
    StackTest stack;
    stack.AddParticle(std::tuple{-99.});

    StackTestView view(stack.first());
    view.AddSecondary(std::tuple{-2.});
    view.AddSecondary(std::tuple{-1.});
    view.AddSecondary(std::tuple{1.});

    auto p1 = view.begin();
    auto p2 = p1 + 1;

    CHECK(p1.GetData() == -2.);
    CHECK(p2.GetData() == -1.);

    view.Copy(p1, p2);

    CHECK(p1.GetData() == -2);
    CHECK(p2.GetData() == -2);
  }
}
