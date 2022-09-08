/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

//#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/sophia/ParticleConversion.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <SetupTestEnvironment.hpp>
#include <catch2/catch.hpp>
#include <tuple>

/*
  NOTE, WARNING, ATTENTION

  The sibyll/Random.hpp implements the hook of sibyll to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below in multiple "tests" and
  link them togehter, it will fail.
 */
#include <corsika/modules/sophia/Random.hpp>

using namespace corsika;
using namespace corsika::sophia;

using DummyEnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
using DummyEnvironment = Environment<DummyEnvironmentInterface>;

TEST_CASE("Sophia", "modules") {

  logging::set_level(logging::level::info);

  SECTION("Sophia -> Corsika") {
    CHECK(Code::Electron ==
          corsika::sophia::convertFromSophia(corsika::sophia::SophiaCode::Electron));
  }

  SECTION("Corsika -> Sophia") {
    CHECK(corsika::sophia::convertToSophia(Electron::code) ==
          corsika::sophia::SophiaCode::Electron);
    CHECK(corsika::sophia::convertToSophiaRaw(Proton::code) == 13);    
  }

  SECTION("canInteractInSophia") {

    CHECK(corsika::sophia::canInteract(Code::Photon));
    CHECK_FALSE(corsika::sophia::canInteract(Code::XiCPlus));

    CHECK_FALSE(corsika::sophia::canInteract(Code::Electron));
    
    CHECK_FALSE(corsika::sophia::canInteract(Code::Iron));
    CHECK_FALSE(corsika::sophia::canInteract(Code::Helium));
  }

  // SECTION("cross-section type") {
  //   CHECK(corsika::sophia::getSophiaXSCode(Code::Proton) == 1);
  //   CHECK(corsika::sophia::getSophiaXSCode(Code::Electron) == 0);
  // }

  SECTION("sophia mass") {
    CHECK_FALSE(corsika::sophia::getSophiaMass(Code::Electron) == 0_GeV);
    // Nucleus not a particle
    CHECK_THROWS(corsika::sophia::getSophiaMass(Code::Iron));
    // Higgs not a particle in Sophia
    CHECK_THROWS(corsika::sophia::getSophiaMass(Code::H0));
  }
}
