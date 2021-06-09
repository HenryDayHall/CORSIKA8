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
#include <boost/filesystem.hpp>

/*
  NOTE, WARNING, ATTENTION

  The sibyll/Random.hpp implements the hook of sibyll to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below in multiple "tests" and
  link them togehter, it will fail.
 */
#include <corsika/modules/qgsjetII/Random.hpp>

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

  logging::set_level(logging::level::info);

  SECTION("check CORSIKA_DATA") {

    const char* CORSIKA_DATA = std::getenv("CORSIKA_DATA");
    // these CHECKS are needed:
    CHECK(CORSIKA_DATA != 0);
    CHECK(boost::filesystem::is_directory(boost::filesystem::path(CORSIKA_DATA) /
                                          "QGSJetII"));
    CORSIKA_LOG_INFO(
        "data: {}"
        " isDir: {}"
        "/QGSJetII",
        CORSIKA_DATA, boost::filesystem::is_directory(CORSIKA_DATA));
  }
}

TEST_CASE("QgsjetII", "[processes]") {

  logging::set_level(logging::level::info);

  SECTION("Corsika -> QgsjetII") {
    CHECK(corsika::qgsjetII::convertToQgsjetII(PiMinus::code) ==
          corsika::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(corsika::qgsjetII::convertToQgsjetIIRaw(Proton::code) == 2);
  }

  SECTION("QgsjetII -> Corsika") {
    CHECK(Code::PiPlus == corsika::qgsjetII::convertFromQgsjetII(
                              corsika::qgsjetII::QgsjetIICode::PiPlus));
    CHECK_THROWS(
        corsika::qgsjetII::convertFromQgsjetII(corsika::qgsjetII::QgsjetIICode::Unknown));
  }

  SECTION("Corsika -> QgsjetII") {
    CHECK(corsika::qgsjetII::convertToQgsjetII(Code::PiMinus) ==
          corsika::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(corsika::qgsjetII::convertToQgsjetIIRaw(Code::Proton) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    CHECK(corsika::qgsjetII::canInteract(Code::Proton));
    CHECK(corsika::qgsjetII::canInteract(Code::KPlus));
    CHECK(corsika::qgsjetII::canInteract(Code::Nucleus));
    // CHECK(corsika::qgsjetII::canInteract(Helium::getCode()));

    CHECK_FALSE(corsika::qgsjetII::canInteract(Code::EtaC));
    CHECK_FALSE(corsika::qgsjetII::canInteract(Code::SigmaC0));
  }

  SECTION("cross-section type") {

    CHECK(corsika::qgsjetII::getQgsjetIIXSCode(Code::Neutron) ==
          corsika::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(corsika::qgsjetII::getQgsjetIIXSCode(Code::K0Long) ==
          corsika::qgsjetII::QgsjetIIXSClass::Kaons);
    CHECK(corsika::qgsjetII::getQgsjetIIXSCode(Code::Proton) ==
          corsika::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(corsika::qgsjetII::getQgsjetIIXSCode(Code::PiMinus) ==
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

  logging::set_level(logging::level::info);

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  RNGManager<>::getInstance().registerRandomStream("qgsjet");

  SECTION("InteractionInterface") {

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Proton, 0, 0, 110_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto particle = stackPtr->first();
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    corsika::qgsjetII::Interaction model;
    model.doInteraction(view);
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

  SECTION("InteractionInterface Nuclei") {

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Nucleus, 60, 30, 20100_GeV,
        (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto particle = stackPtr->first();
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    corsika::qgsjetII::Interaction model;
    model.doInteraction(view); // this also should produce some fragments
    CHECK(view.getSize() == Approx(350).margin(100)); // this is not physics validation
    int countFragments = 0;
    for (auto const& sec : view) { countFragments += (sec.getPID() == Code::Nucleus); }
    CHECK(countFragments == Approx(4).margin(2)); // this is not physics validation
    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);

    CHECK(length / (1_g / square(1_cm)) ==
          Approx(12).margin(2)); // this is not physics validation
  }

  SECTION("Heavy nuclei") {

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Nucleus, 1000, 1000, 1100_GeV,
        (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto particle = stackPtr->first();
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    corsika::qgsjetII::Interaction model;

    CHECK_THROWS(
        model.getCrossSection(Code::Nucleus, Code::Nucleus, 100_GeV, 10., 1000.));
    CHECK_THROWS(
        model.getCrossSection(Code::Nucleus, Code::Nucleus, 100_GeV, 1000., 10.));
    CHECK_THROWS(model.doInteraction(view));
    CHECK_THROWS(model.getInteractionLength(particle));
  }

  SECTION("Allowed Particles") {
    { // electron
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Electron, 0, 0, 100_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
          *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      auto particle = stackPtr->first();
      corsika::qgsjetII::Interaction model;
      GrammageType const length = model.getInteractionLength(particle);
      CHECK(length / (1_g / square(1_cm)) == std::numeric_limits<double>::infinity());
    }
    { // pi0 is internally converted into pi+/pi-
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Pi0, 0, 0, 1000_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
          *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::Interaction model;
      model.doInteraction(view);
      CHECK(view.getSize() == Approx(10).margin(8)); // this is not physics validation
    }
    { // rho0 is internally converted into pi-/pi+
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Rho0, 0, 0, 1000_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
          *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::Interaction model;
      model.doInteraction(view);
      CHECK(view.getSize() == Approx(25).margin(20)); // this is not physics validation
    }
    { // Lambda is internally converted into neutron
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Lambda0, 0, 0, 100_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
          *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::Interaction model;
      model.doInteraction(view);
      CHECK(view.getSize() == Approx(25).margin(20)); // this is not physics validation
    }
    { // AntiLambda is internally converted into anti neutron
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Lambda0Bar, 0, 0, 1000_GeV,
          (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::Interaction model;
      model.doInteraction(view);
      CHECK(view.getSize() == Approx(70).margin(67)); // this is not physics validation
    }
  }
}
