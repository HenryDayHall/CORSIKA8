
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

#include <iostream>
using namespace std;
using namespace corsika;

TEST_CASE("Sibyll", "[processes]") {

  SECTION("Sibyll -> Corsika") {
    REQUIRE(corsika::particles::Electron::GetCode() ==
            process::sibyll::ConvertFromSibyll(process::sibyll::SibyllCode::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    REQUIRE(process::sibyll::ConvertToSibyll(corsika::particles::Electron::GetCode()) ==
            process::sibyll::SibyllCode::Electron);
    REQUIRE(process::sibyll::ConvertToSibyllRaw(corsika::particles::Proton::GetCode()) ==
            13);
  }

  SECTION("KnownBySibyll") {
    REQUIRE(process::sibyll::KnownBySibyll(corsika::particles::Electron::GetCode()));

    REQUIRE_FALSE(
        process::sibyll::KnownBySibyll(corsika::particles::XiPrimeC0::GetCode()));
  }

  SECTION("canInteractInSibyll") {

    REQUIRE(process::sibyll::CanInteract(corsika::particles::Proton::GetCode()));
    REQUIRE(process::sibyll::CanInteract(corsika::particles::Code::XiCPlus));

    REQUIRE_FALSE(process::sibyll::CanInteract(corsika::particles::Electron::GetCode()));
    REQUIRE_FALSE(process::sibyll::CanInteract(corsika::particles::SigmaC0::GetCode()));
  }
  
  SECTION("canInteractInSibyll") {

    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::Electron) == 0);
    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::K0Long) == 3);
    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::SigmaPlus) == 1);
    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::PiMinus) == 2);
  }
}
