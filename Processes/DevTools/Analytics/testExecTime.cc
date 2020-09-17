/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <corsika/process/devtools/ExecTime.h>

#include <corsika/process/devtools/DummyBoundaryCrossingProcess.h>
#include <corsika/process/devtools/DummyContinuousProcess.h>
#include <corsika/process/devtools/DummyDecayProcess.h>
#include <corsika/process/devtools/DummyInteractionProcess.h>
#include <corsika/process/devtools/DummySecondariesProcess.h>

using namespace corsika::process;
using namespace corsika::process::devtools;

TEST_CASE("ContinuousProcess interface", "[proccesses][DevTools ExecTime]") {

  ExecTime<DummyBoundaryCrossingProcess<100>> execTime;
  int tmp = 0;

  SECTION("BoundaryCrossing") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(execTime.DoBoundaryCrossing(tmp, 0, 0) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(100).margin(1));
  }
}
