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

#include <random>

using namespace corsika::process;
using namespace corsika::process::devtools;

TEST_CASE("ContinuousProcess interface", "[proccesses][DevTools ExecTime]") {

  ExecTime<DummyBoundaryCrossingProcess<50>> execTime;
  int tmp = 0;

  SECTION("BoundaryCrossing") {
    auto start = std::chrono::steady_clock::now();
    REQUIRE(execTime.DoBoundaryCrossing(tmp, 0, 0) == EProcessReturn::eOk);
    auto end = std::chrono::steady_clock::now();
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() ==
            Approx(50).margin(1));

    for (int i = 0; i < 100; i++) execTime.DoBoundaryCrossing(tmp, 0, 0);

    REQUIRE(execTime.mean() == Approx(50 * 1000).margin(1 * 1000));

    REQUIRE(execTime.sumTime() == Approx(50 * 100 * 1000).margin(100 * 1000));

    REQUIRE(execTime.var() == Approx(0).margin(20000));
  }

  SECTION("TestMeanAlgo") {
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(10000.0, 200.0);

    double fStart;
    double fElapsedSum;
    double fMean;
    double fMean2;
    long long fMin;
    long long fMax;
    long long fN;

    for (int i = 0; i < 1000000; i++) {
      auto timeDiv = distribution(generator);

      fElapsedSum += timeDiv;
      fN = fN + 1;

      if (fMax < timeDiv) fMax = timeDiv;

      if (timeDiv < fMin) fMin = timeDiv;

      double delta = timeDiv - fMean;
      fMean += delta / static_cast<double>(fN);

      double delta2 = timeDiv - fMean;

      fMean2 += delta * delta2;
    }

    REQUIRE(fMean2 / fN == Approx(200*200).margin(200)); // Varianz
    REQUIRE(fMean == Approx(10000).margin(10));
  }
}
