/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/random/ExponentialDistribution.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/units/PhysicalUnits.h>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>

using namespace corsika::random;

SCENARIO("random-number streams can be registered and retrieved") {
  GIVEN("a RNGManager") {
    RNGManager& rngManager = RNGManager::GetInstance();

    WHEN("a sequence is registered by name") {
      rngManager.RegisterRandomStream("stream_A");

      THEN("the sequence can be retrieved") {
        REQUIRE_NOTHROW(rngManager.GetRandomStream("stream_A"));
      }

      THEN("an unknown sequence cannot be retrieved") {
        REQUIRE_THROWS(rngManager.GetRandomStream("stream_UNKNOWN"));
      }

      // seeding not covered yet
    }
  }
}

TEST_CASE("UniformRealDistribution") {
  using namespace corsika::units::si;
  std::mt19937 rng;

  corsika::random::UniformRealDistribution<LengthType> dist(1_m, 2_m);

  SECTION("range") {
    corsika::random::UniformRealDistribution<LengthType> dist(1_m, 2_m);

    LengthType min =
        +1_m * std::numeric_limits<typename LengthType::value_type>::infinity();
    LengthType max =
        -1_m * std::numeric_limits<typename LengthType::value_type>::infinity();

    for (int i{0}; i < 1'000'000; ++i) {
      LengthType x = dist(rng);
      min = std::min(min, x);
      max = std::max(max, x);
    }

    CHECK(min / 1_m == Approx(1.));
    CHECK(max / 2_m == Approx(1.));
  }

  SECTION("range") {
    corsika::random::UniformRealDistribution<LengthType> dist(18_cm);

    LengthType min =
        +1_m * std::numeric_limits<typename LengthType::value_type>::infinity();
    LengthType max =
        -1_m * std::numeric_limits<typename LengthType::value_type>::infinity();

    for (int i{0}; i < 1'000'000; ++i) {
      LengthType x = dist(rng);
      min = std::min(min, x);
      max = std::max(max, x);
    }

    CHECK(min / 1_m == Approx(0.).margin(1e-3));
    CHECK(max / 18_cm == Approx(1.));
  }
}

TEST_CASE("ExponentialDistribution") {
  using namespace corsika::units::si;
  std::mt19937 rng;

  auto const beta = 15_m;

  corsika::random::ExponentialDistribution dist(beta);

  SECTION("mean") {
    std::remove_const<decltype(beta)>::type mean = beta * 0;

    int constexpr N = 1'000'000;

    for (int i{0}; i < N; ++i) {
      decltype(beta) x = dist(rng);
      mean += x / N;
    }

    CHECK(mean / beta == Approx(1).margin(1e-2));
  }
}
