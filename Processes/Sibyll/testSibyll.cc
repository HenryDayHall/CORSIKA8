
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/units/PhysicalUnits.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("Sibyll", "[processes]") {

  SECTION("ParticleConversion") {
    REQUIRE(corsika::particles::Electron::GetCode() ==
            process::sibyll::Sibyll2Corsika.at(process::sibyll::PID::E_MINUS));
  }

  SECTION("Data") {
    REQUIRE(corsika::particles::GetName(process::sibyll::Sibyll2Corsika.at(
                process::sibyll::PID::E_PLUS)) == "e+");
  }

  SECTION("bla") {}

  SECTION("blubb") {}
}
