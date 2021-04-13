/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/stack/CombinedStack.hpp>
#include <corsika/stack/DummyStack.hpp>
#include <corsika/stack/WeightStackExtension.hpp>

using namespace corsika;

#include <catch2/catch.hpp>

#include <iostream>
using namespace std;

// the Weight stack:
template <typename TStackIter>
using DummyWeightDataInterface =
    typename weights::MakeWeightDataInterface<TStackIter>::type;

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithGeometryInterface =
    CombinedParticleInterface<dummy_stack::DummyStack::pi_type, DummyWeightDataInterface,
                              TStackIter>;

using TestStack =
    CombinedStack<typename dummy_stack::DummyStack::stack_implementation_type,
                  weights::WeightData, StackWithGeometryInterface>;

TEST_CASE("WeightStackExtension", "[stack]") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  dummy_stack::NoData noData;

  SECTION("write weights") {

    double const weight = 5.1;

    TestStack s;
    s.addParticle(std::make_tuple(noData), std::tuple<double>{weight});

    CHECK(s.getEntries() == 1);
  }

  SECTION("write/read weights") {
    double const weight = 15;

    TestStack s;
    auto p = s.addParticle(std::make_tuple(noData));
    p.setWeight(weight);
    CHECK(s.getEntries() == 1);

    const auto pout = s.getNextParticle();
    CHECK(pout.getWeight() == weight);
  }

  SECTION("stack fill and cleanup") {

    double const weight = 16;

    TestStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      auto p = s.addParticle(std::tuple<dummy_stack::NoData>{noData});
      p.setWeight(weight);
    }

    CHECK(s.getEntries() == 99);
    double v = 0;
    for (int i = 0; i < 99; ++i) {
      auto p = s.getNextParticle();
      v += p.getWeight();
      p.erase();
    }
    CHECK(v == Approx(99 * weight));
    CHECK(s.getEntries() == 0);
  }
}
