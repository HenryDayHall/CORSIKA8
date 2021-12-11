/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/QGSJetII.hpp>

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

TEST_CASE("QgsjetII", "[processes]") {

  logging::set_level(logging::level::info);
  RNGManager<>::getInstance().registerRandomStream("qgsjet");

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

  SECTION("valid") {

    corsika::qgsjetII::InteractionModel model;

    CHECK_FALSE(model.isValid(Code::Electron, Code::Proton, 1_TeV));
    CHECK_FALSE(model.isValid(Code::Proton, Code::Electron, 1_TeV));
    CHECK_FALSE(model.isValid(Code::Proton, Code::Proton, 1_GeV));
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

TEST_CASE("QgsjetIIInterface", "interaction,processes") {

  logging::set_level(logging::level::info);

  RNGManager<>::getInstance().registerRandomStream("qgsjet");

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  SECTION("InteractionInterface") {

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Proton, 110_GeV, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    corsika::qgsjetII::InteractionModel model;
    model.doInteraction(view, Code::Proton, Code::Oxygen,
                        {sqrt(static_pow<2>(110_GeV) + static_pow<2>(Proton::mass)),
                         MomentumVector{cs, 110_GeV, 0_GeV, 0_GeV}},
                        {Oxygen::mass, MomentumVector{cs, {0_eV, 0_eV, 0_eV}}});

    /* **********************************
     As it turned out already two times (#291 and #307) that the detailed output of
     QGSJetII event generation depends on the gfortran version used. This is not reliable
     and cannot be tested in a unit test here. One related problem was already found
    (#291) and is realted to undefined behaviour in the evaluation of functions in logical
     expressions. It is not clear if #307 is the same issue.

     CHECK(view.getSize() == 14);
     CHECK(sumCharge(view) == 2);
    *********************************** */
    auto const secMomSum = sumMomentum(view, projectileMomentum.getCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).getNorm() / projectileMomentum.getNorm() ==
          Approx(0).margin(1e-2));
  }

  SECTION("InteractionInterface Nuclei") {

    HEPEnergyType const P0 = 20100_GeV;
    MomentumVector const plab = MomentumVector(cs, {P0, 0_eV, 0_eV});
    Code const pid = get_nucleus_code(60, 30);
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        pid, P0, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    setup::StackView& view = *(secViewPtr.get());

    HEPEnergyType const Elab = sqrt(static_pow<2>(P0) + static_pow<2>(get_mass(pid)));
    FourMomentum const projectileP4(Elab, plab);
    FourMomentum const targetP4(Oxygen::mass, MomentumVector(cs, {0_eV, 0_eV, 0_eV}));
    view.clear();

    corsika::qgsjetII::InteractionModel model;
    model.doInteraction(view, pid, Code::Oxygen, projectileP4,
                        targetP4); // this also should produce some fragments
    CHECK(view.getSize() == Approx(300).margin(150)); // this is not physics validation
    int countFragments = 0;
    for (auto const& sec : view) { countFragments += (is_nucleus(sec.getPID())); }
    CHECK(countFragments == Approx(4).margin(2)); // this is not physics validation
  }

  SECTION("Heavy nuclei") {

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        get_nucleus_code(1000, 1000), 1100_GeV,
        (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    setup::StackView& view = *(secViewPtr.get());
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    corsika::qgsjetII::InteractionModel model;

    FourMomentum const aP4(100_GeV, {cs, 99_GeV, 0_GeV, 0_GeV});
    FourMomentum const bP4(1_TeV, {cs, 0.9_TeV, 0_GeV, 0_GeV});

    CHECK(model.getCrossSection(get_nucleus_code(10, 5), get_nucleus_code(1000, 500), aP4,
                                bP4) /
              1_mb ==
          Approx(0));
    CHECK(model.getCrossSection(Code::Nucleus, Code::Nucleus, aP4, bP4) / 1_mb ==
          Approx(0));
    CHECK_THROWS(
        model.doInteraction(view, get_nucleus_code(1000, 500), Code::Oxygen, aP4, bP4));
  }

  SECTION("Allowed Particles") {

    { // pi0 is internally converted into pi+/pi-
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Pi0, 1000_GeV, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::InteractionModel model;
      model.doInteraction(view, Code::Pi0, Code::Oxygen,
                          {sqrt(static_pow<2>(1_TeV) + static_pow<2>(Pi0::mass)),
                           MomentumVector{cs, 1_TeV, 0_GeV, 0_GeV}},
                          {Oxygen::mass, MomentumVector{cs, 0_eV, 0_eV, 0_eV}});
      CHECK(view.getSize() == Approx(10).margin(8)); // this is not physics validation
    }
    { // rho0 is internally converted into pi-/pi+
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Rho0, 1000_GeV, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::InteractionModel model;
      model.doInteraction(view, Code::Rho0, Code::Oxygen,
                          {sqrt(static_pow<2>(1_TeV) + static_pow<2>(Rho0::mass)),
                           MomentumVector{cs, 1_TeV, 0_GeV, 0_GeV}},
                          {Oxygen::mass, MomentumVector{cs, 0_eV, 0_eV, 0_eV}});
      CHECK(view.getSize() == Approx(50).margin(20)); // this is not physics validation
    }
    { // Lambda is internally converted into neutron
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Lambda0, 100_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
          *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::InteractionModel model;
      model.doInteraction(view, Code::Lambda0, Code::Oxygen,
                          {sqrt(static_pow<2>(100_GeV) + static_pow<2>(Lambda0::mass)),
                           MomentumVector{cs, 100_GeV, 0_GeV, 0_GeV}},
                          {Oxygen::mass, MomentumVector{cs, 0_eV, 0_eV, 0_eV}});
      CHECK(view.getSize() == Approx(25).margin(20)); // this is not physics validation
    }
    { // AntiLambda is internally converted into anti neutron
      auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
          Code::Lambda0Bar, 1000_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
          *csPtr);
      [[maybe_unused]] setup::StackView& view = *(secViewPtr.get());
      [[maybe_unused]] auto particle = stackPtr->first();
      corsika::qgsjetII::InteractionModel model;
      model.doInteraction(view, Code::Lambda0Bar, Code::Oxygen,
                          {sqrt(static_pow<2>(1_TeV) + static_pow<2>(Lambda0Bar::mass)),
                           MomentumVector{cs, 1_TeV, 0_GeV, 0_GeV}},
                          {Oxygen::mass, MomentumVector{cs, 0_eV, 0_eV, 0_eV}});
      CHECK(view.getSize() == Approx(70).margin(67)); // this is not physics validation
    }
  }
}
