/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/Epos.hpp>
//#include <corsika/modules/epos/ParticleConversion.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <catch2/catch.hpp>
#include <tuple>

/*
  NOTE, WARNING, ATTENTION

  The epos/Random.hpp implements the hook of epos to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below in multiple "tests" and
  link them togehter, it will fail.
 */
#include <corsika/modules/epos/Random.hpp>

using namespace corsika;
using namespace corsika::epos;

TEST_CASE("Epos", "[processes]") {

  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");
  logging::set_level(logging::level::trace);

  SECTION("Epos -> Corsika") {
  }

  SECTION("Corsika -> Epos") {
  }

  SECTION("canInteractInEpos") {
  }

  SECTION("cross-section type") {
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

TEST_CASE("EposInterface", "[processes]") {

  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");
  logging::set_level(logging::level::trace);

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;

  RNGManager::getInstance().registerRandomStream("epos");

  SECTION("InteractionInterface - random number"){
    auto const rndm = ::epos::rangen_();
    CHECK(rndm>0);
    CHECK(rndm<1);
  }
  
  SECTION("InteractionInterface - valid targets") {

    Interaction model;
    // eposlhc accepts protons or nuclei with 4<=A<=18 as targets
    CHECK_FALSE(model.isValidTarget(Code::Electron));
    CHECK(model.isValidTarget(Code::Hydrogen));
    CHECK_FALSE(model.isValidTarget(Code::Deuterium));
    CHECK(model.isValidTarget(Code::Helium));
    CHECK_FALSE(model.isValidTarget(Code::Helium3));
    CHECK_FALSE(model.isValidTarget(Code::Iron));
    CHECK(model.isValidTarget(Code::Oxygen));

    //  hydrogen target == proton target == neutron target
    auto const [xs_prod_pp, xs_ela_pp] =
        model.getCrossSection(Code::Proton, Code::Proton, 100_GeV);
    auto const [xs_prod_pn, xs_ela_pn] =
        model.getCrossSection(Code::Proton, Code::Neutron, 100_GeV);
    auto const [xs_prod_pHydrogen, xs_ela_pHydrogen] =
        model.getCrossSection(Code::Proton, Code::Hydrogen, 100_GeV);
    CHECK(xs_prod_pp == xs_prod_pHydrogen);
    CHECK(xs_prod_pp == xs_prod_pn);
    CHECK(xs_ela_pp == xs_ela_pHydrogen);
    CHECK(xs_ela_pn == xs_ela_pHydrogen);
  }

  SECTION("InteractionInterface - low energy") {

    const HEPEnergyType P0 = 60_GeV;
    auto [stack, viewPtr] = setup::testing::setup_stack(
        Code::Proton, 0, 0, P0, (setup::Environment::BaseNodeType* const)nodePtr, cs);
    MomentumVector plab =
        MomentumVector(cs, {P0, 0_eV, 0_eV}); // this is secret knowledge about setupStack
    setup::StackView& view = *viewPtr;

    auto particle = stack->first();

    Interaction model;
    model.doInteraction(view);

    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);
    CHECK(length / 1_g * 1_cm * 1_cm == Approx(88.7).margin(0.1));

  }

}
