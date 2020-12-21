/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/qgsjetII/Interaction.hpp>
#include <corsika/modules/qgsjetII/ParticleConversion.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <catch2/catch.hpp>

#include <string>
#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>

using namespace corsika;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;
  for (auto const& p : view) { totalCharge += get_charge_number(p.getPID()); }
  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, CoordinateSystemPtr const& vCS) {
  Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};
  for (auto const& p : view) { sum += p.getMomentum(); }
  return sum;
}

TEST_CASE("CORSIKA_DATA", "[processes]") {

  SECTION("check CORSIKA_DATA") {

    const char* data = std::getenv("CORSIKA_DATA");
    // these REQUIRES are needed:
    REQUIRE(data != 0);
    REQUIRE(std::experimental::filesystem::is_directory(
        std::experimental::filesystem::path(std::string(data) + "/QGSJetII")));
    std::cout << "data: " << data << " isDir: "
              << std::experimental::filesystem::is_directory(std::string(data) +
                                                             "/QGSJetII")
              << std::endl;
  }
}

TEST_CASE("QgsjetII", "[processes]") {

  SECTION("Corsika -> QgsjetII") {
    CHECK(corsika::qgsjetII::convertToQgsjetII(PiMinus::code) ==
          corsika::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(corsika::qgsjetII::convertToQgsjetIIRaw(Proton::code) == 2);
  }

  SECTION("QgsjetII -> Corsika") {
    REQUIRE(Code::PiPlus == corsika::qgsjetII::convertFromQgsjetII(
                                corsika::qgsjetII::QgsjetIICode::PiPlus));
  }

  SECTION("Corsika -> QgsjetII") {
    REQUIRE(corsika::qgsjetII::convertToQgsjetII(Code::PiMinus) ==
            corsika::qgsjetII::QgsjetIICode::PiMinus);
    REQUIRE(corsika::qgsjetII::convertToQgsjetIIRaw(Code::Proton) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    REQUIRE(corsika::qgsjetII::canInteract(Code::Proton));
    REQUIRE(corsika::qgsjetII::canInteract(Code::KPlus));
    REQUIRE(corsika::qgsjetII::canInteract(Code::Nucleus));
    // REQUIRE(corsika::qgsjetII::canInteract(Helium::getCode()));

    REQUIRE_FALSE(corsika::qgsjetII::canInteract(Code::EtaC));
    REQUIRE_FALSE(corsika::qgsjetII::canInteract(Code::SigmaC0));
  }

  SECTION("cross-section type") {

    REQUIRE(corsika::qgsjetII::getQgsjetIIXSCode(Code::Neutron) ==
            corsika::qgsjetII::QgsjetIIXSClass::Baryons);
    REQUIRE(corsika::qgsjetII::getQgsjetIIXSCode(Code::K0Long) ==
            corsika::qgsjetII::QgsjetIIXSClass::Kaons);
    REQUIRE(corsika::qgsjetII::getQgsjetIIXSCode(Code::Proton) ==
            corsika::qgsjetII::QgsjetIIXSClass::Baryons);
    REQUIRE(corsika::qgsjetII::getQgsjetIIXSCode(Code::PiMinus) ==
            corsika::qgsjetII::QgsjetIIXSClass::LightMesons);
  }
}

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <SetupTestEnvironment.hpp>
#include <SetupTestStack.hpp>

TEST_CASE("QgsjetIIInterface", "[processes]") {

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  RNGManager::getInstance().registerRandomStream("qgsjet");

  SECTION("InteractionInterface") {

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Proton, 0, 0, 110_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto particle = stackPtr->first();
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    corsika::qgsjetII::Interaction model;
    model.doInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);

    CHECK(length / (1_g / square(1_cm)) == Approx(93.04).margin(0.1));

    /***********************************
     It as turned out already two times (#291 and #307) that the detailed output of
    QGSJetII event generation depends on the gfortran version used. This is not reliable
    and cannot be tested in a unit test here. One related problem was already found (#291)
    and is realted to undefined behaviour in the evaluation of functions in logical
    expressions. It is not clear if #307 is the same issue.

     CHECK(view.getSize() == 14);
     CHECK(sumCharge(view) == 2);
    ************************************/
    auto const secMomSum = sumMomentum(view, projectileMomentum.getCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).getNorm() / projectileMomentum.getNorm() ==
          Approx(0).margin(1e-2));
  }
}
