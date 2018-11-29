
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

  SECTION("Sibyll -> Corsika") {
    REQUIRE(corsika::particles::Electron::GetCode() ==
            process::sibyll::ConvertFromSibyll(process::sibyll::Code::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    REQUIRE(process::sibyll::ConvertToSibyll(corsika::particles::Electron::GetCode()) ==
            process::sibyll::Code::Electron);
  }

  SECTION("handledBySibyll") {
    REQUIRE(process::sibyll::HandledBySibyll(corsika::particles::Electron::GetCode()));

    REQUIRE_FALSE(
        process::sibyll::HandledBySibyll(corsika::particles::XiPrimeC0::GetCode()));
  }
}
