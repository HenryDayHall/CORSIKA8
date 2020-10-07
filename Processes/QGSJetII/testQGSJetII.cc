/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/qgsjetII/Interaction.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>

#include <corsika/random/RNGManager.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>

using namespace corsika;
using namespace corsika::process::qgsjetII;
using namespace corsika::units::si;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;

  for (auto const& p : view) { totalCharge += particles::GetChargeNumber(p.GetPID()); }

  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

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
    CHECK(process::qgsjetII::ConvertToQgsjetII(particles::PiMinus::GetCode()) ==
          process::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(process::qgsjetII::ConvertToQgsjetIIRaw(particles::Proton::GetCode()) == 2);
  }

  SECTION("QgsjetII -> Corsika") {
    CHECK(particles::PiPlus::GetCode() == process::qgsjetII::ConvertFromQgsjetII(
                                              process::qgsjetII::QgsjetIICode::PiPlus));
  }

  SECTION("Corsika -> QgsjetII") {
    CHECK(process::qgsjetII::ConvertToQgsjetII(particles::PiMinus::GetCode()) ==
          process::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(process::qgsjetII::ConvertToQgsjetIIRaw(particles::Proton::GetCode()) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    CHECK(process::qgsjetII::CanInteract(particles::Proton::GetCode()));
    CHECK(process::qgsjetII::CanInteract(particles::Code::KPlus));
    CHECK(process::qgsjetII::CanInteract(particles::Nucleus::GetCode()));
    // CHECK(process::qgsjetII::CanInteract(particles::Helium::GetCode()));

    CHECK_FALSE(process::qgsjetII::CanInteract(particles::EtaC::GetCode()));
    CHECK_FALSE(process::qgsjetII::CanInteract(particles::SigmaC0::GetCode()));
  }

  SECTION("cross-section type") {

    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::Neutron) ==
          process::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::K0Long) ==
          process::qgsjetII::QgsjetIIXSClass::Kaons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::Proton) ==
          process::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::PiMinus) ==
          process::qgsjetII::QgsjetIIXSClass::LightMesons);
  }
}

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/UniformMediumType.h>
#include <corsika/environment/UniformMagneticField.h>

using namespace corsika::units::si;
using namespace corsika::units;
using namespace corsika;

TEST_CASE("QgsjetIIInterface", "[processes]") {

  auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  corsika::random::RNGManager::GetInstance().RegisterRandomStream("qgsjet");

  SECTION("InteractionInterface") {

    auto [stackPtr, secViewPtr] =
      setup::testing::setupStack(particles::Code::Proton, 0,0, 110_GeV, nodePtr, *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto particle = stackPtr->first();
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    Interaction model;

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(view);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);

    CHECK(length / (1_g / square(1_cm)) == Approx(93.04).margin(0.1));

    /***********************************
     It as turned out already two times (#291 and #307) that the detailed output of
    QGSJetII event generation depends on the gfortran version used. This is not reliable
    and cannot be tested in a unit test here. One related problem was already found (#291)
    and is realted to undefined behaviour in the evaluation of functions in logical
    expressions. It is not clear if #307 is the same issue.

     CHECK(view.GetSize() == 14);
     CHECK(sumCharge(view) == 2);
    ************************************/
    auto const secMomSum = sumMomentum(view, projectileMomentum.GetCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
          Approx(0).margin(1e-2));
  }
}
