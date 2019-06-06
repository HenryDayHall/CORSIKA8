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

    StackTestView view(particle);
    REQUIRE(view.GetSize() == 0);

    {
      auto proj = view.GetProjectile();
      REQUIRE(proj.GetData() == particle.GetData());
      proj.AddSecondary(std::tuple{4.4});
    }

    view.AddSecondary(std::tuple{4.5});
    view.AddSecondary(std::tuple{4.6});

    REQUIRE(view.GetSize() == 3);
    REQUIRE(s.GetSize() == 5);
    REQUIRE(!view.IsEmpty());

    auto sumView = [](const StackTestView& stack) {
      double value = 0;
      for (const auto& p : stack) { value += p.GetData(); }
      return value;
    };

    REQUIRE(sum(s) == sumS + 4.4 + 4.5 + 4.6);
    REQUIRE(sumView(view) == 4.4 + 4.5 + 4.6);

    view.DeleteLast();
    REQUIRE(view.GetSize() == 2);
    REQUIRE(s.GetSize() == 4);

    REQUIRE(sum(s) == sumS + 4.4 + 4.5);
    REQUIRE(sumView(view) == 4.4 + 4.5);

    auto pDel = view.GetNextParticle();
    view.Delete(pDel);
    REQUIRE(view.GetSize() == 1);
    REQUIRE(s.GetSize() == 3);

    REQUIRE(sum(s) == sumS + 4.4 + 4.5 - pDel.GetData());
    REQUIRE(sumView(view) == 4.4 + 4.5 - pDel.GetData());

    view.Delete(view.GetNextParticle());
    REQUIRE(sum(s) == sumS);
    REQUIRE(sumView(view) == 0);
    REQUIRE(view.IsEmpty());

    {
      auto proj = view.GetProjectile();
      REQUIRE(proj.GetData() == particle.GetData());
    }
  }

  SECTION("secondary view, construct from ParticleType") {
    StackTest s;
    REQUIRE(s.GetSize() == 0);
    s.AddParticle(std::tuple{9.9});
    s.AddParticle(std::tuple{8.8});

    auto iterator = s.GetNextParticle();
    typename StackTest::ParticleType& particle = iterator; // as in corsika::Cascade

    StackTestView view(particle);
    REQUIRE(view.GetSize() == 0);

    view.AddSecondary(std::tuple{4.4});

    REQUIRE(view.GetSize() == 1);
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

      CHECK(stack.GetSize() == 6); // -99, 0, -2, -1, 1, 2
      CHECK(view.GetSize() == 4);  // -2, -1, 1, 2

      // now delete all negative entries, i.e. -1 and -2
      auto p = view.begin();
      while (p != view.end()) {
        auto data = p.GetData();
        if (data < 0) {
          p.Delete();
        } else {
          ++p;
        }
      }
      CHECK(stack.GetSize() == 4); // -99, 0, 2, 1 (order changes during deletion)
      CHECK(view.GetSize() == 2);  // 2, 1
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
        if (data < 0) {
          p.Delete();
        } else {
          ++p;
        }
      }

      // stack should contain -99, 0, 2, 1, [2, 1]
      // view should contain 1, 2

      CHECK(stack.GetSize() == 6);
    }
  }
}
