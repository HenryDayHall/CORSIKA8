/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/Sophia.hpp>
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
    CHECK_THROWS(convertFromSophia(corsika::sophia::SophiaCode::Unknown));
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

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>

#include <SetupTestEnvironment.hpp>
#include <SetupTestStack.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/UniformMagneticField.hpp>

template <typename TStackView>
auto sumMomentum(TStackView const& view, CoordinateSystemPtr const& vCS) {
  Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};
  for (auto const& p : view) { sum += p.getMomentum(); }
  return sum;
}

TEST_CASE("SophiaInterface", "modules") {

  logging::set_level(logging::level::debug);

  // the environment and stack should eventually disappear from here
  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;
  { [[maybe_unused]] auto const& env_dummy = env; }

  auto [stack, viewPtr] = setup::testing::setup_stack(
      Code::Photon, 10_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr, cs);
  test::StackView& view = *viewPtr;

  RNGManager<>::getInstance().registerRandomStream("sophia");

  SECTION("InteractionInterface - valid targets/projectiles") {

    corsika::sophia::InteractionModel model;

    CHECK_FALSE(model.isValid(Code::Proton, Code::Electron, 100_GeV));
    CHECK(model.isValid(Code::Photon, Code::Hydrogen, 3_GeV));

    FourMomentum const aP4(100_GeV, {cs, 99_GeV, 0_GeV, 0_GeV});
    FourMomentum const bP4(1_GeV, {cs, 0_GeV, 0_GeV, 0_GeV});
    CHECK(0_mb == model.getCrossSection(Code::Photon, Code::Proton, aP4, bP4));
  }

  SECTION("InteractionInterface - interaction") {
    const HEPEnergyType P0 = 1.2_GeV;
    MomentumVector const plab = MomentumVector(cs, {P0, 0_eV, 0_eV});
    // also print particles after SOPHIA was called
    corsika::sophia::InteractionModel model;
    model.setVerbose(true);
    HEPEnergyType const Elab = P0;
    FourMomentum const projectileP4(Elab, plab);
    FourMomentum const nucleonP4(Proton::mass, MomentumVector(cs, {0_eV, 0_eV, 0_eV}));
    view.clear();
    model.doInteraction(view, Code::Photon, Code::Proton, projectileP4, nucleonP4);

    auto const pSum = sumMomentum(view, cs);
    CHECK(pSum.getComponents(cs).getX() / P0 == Approx(1).margin(0.05));
    CHECK(pSum.getComponents(cs).getY() / 1_GeV == Approx(0).margin(1e-3));
    CHECK(pSum.getComponents(cs).getZ() / 1_GeV == Approx(0).margin(1e-3));
  }
}