/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/example_processors/DummyBoundaryCrossingProcess.h>
#include <corsika/process/example_processors/DummyContinuousProcess.h>
#include <corsika/process/example_processors/DummyDecayProcess.h>
#include <corsika/process/example_processors/DummyInteractionProcess.h>
#include <corsika/process/example_processors/DummySecondariesProcess.h>

#include <corsika/process/ProcessReturn.h>

#include <catch2/catch.hpp>

#include <chrono>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::process::example_processors;

using namespace corsika::units::si;

TEST_CASE("Dummy Processes") {
  DummyBoundaryCrossingProcess<1000> dbc;
  DummyContinuousProcess<1000> dc;
  DummyDecayProcess<1000> dd;
  DummyInteractionProcess<1000> di;
  DummySecondariesProcess<1000> dse;

  int tmp = 0;
  SECTION("BoundaryCrossing") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(dbc.DoBoundaryCrossing(tmp, 0, 0) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));
  }

  SECTION("Continuous") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(dc.DoContinuous(tmp, nullptr) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));

    start = std::chrono::steady_clock::now();
    REQUIRE(dc.MaxStepLength(nullptr, nullptr) == units::si::meter * std::numeric_limits<double>::infinity());
    end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));
  }

   SECTION("Decay") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(dd.DoDecay(tmp) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));

    start = std::chrono::steady_clock::now();
    REQUIRE(dd.GetLifetime(tmp) == units::si::second * std::numeric_limits<double>::infinity());
    end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));
  }

   SECTION("Interaction") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(di.DoInteraction(tmp) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));

    start = std::chrono::steady_clock::now();
    REQUIRE(di.GetInteractionLength(tmp) ==  (units::si::gram / 1_cm / 1_cm) * std::numeric_limits<double>::infinity());
    end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));
  }

   SECTION("Secondaries") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(dse.DoSecondaries(tmp) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(1000).margin(1));
   
  }
}
