/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/Epos.hpp>

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

TEST_CASE("epos", "modules") {

  logging::set_level(logging::level::trace);

  SECTION("epos -> corsika") {
    CHECK(Code::Electron ==
          corsika::epos::convertFromEpos(corsika::epos::EposCode::Electron));
    CHECK(Code::Proton ==
          corsika::epos::convertFromEpos(corsika::epos::EposCode::Proton));
  }

  SECTION("corsika -> epos") {
    CHECK(corsika::epos::convertToEpos(Electron::code) ==
          corsika::epos::EposCode::Electron);
    // check if particle code is correct for common particles that interact (secret epos
    // knowledge)
    CHECK(corsika::epos::convertToEposRaw(Proton::code) == 1120);
    CHECK(corsika::epos::convertToEposRaw(PiPlus::code) == 120);
    CHECK(corsika::epos::convertToEposRaw(KPlus::code) == 130);
  }

  SECTION("canInteractInEpos") {
    CHECK(corsika::epos::canInteract(Code::Proton));
    CHECK_FALSE(corsika::epos::canInteract(Code::Electron));
    CHECK(corsika::epos::canInteract(Code::Nucleus));
    CHECK(corsika::epos::canInteract(Code::Helium));
  }

  SECTION("cross-section type") {
    CHECK(corsika::epos::getEposXSCode(Code::Electron) == 0);
    CHECK(corsika::epos::getEposXSCode(Code::K0Long) == 0);
    CHECK(corsika::epos::getEposXSCode(Code::SigmaPlus) == 0);
    CHECK(corsika::epos::getEposXSCode(Code::KMinus) == 3);
    CHECK(corsika::epos::getEposXSCode(Code::PiMinus) == 1);
    CHECK(corsika::epos::getEposXSCode(Code::Proton) == 2);
    CHECK(corsika::epos::getEposXSCode(Code::Helium) == 2);
    CHECK(corsika::epos::getEposXSCode(Code::Nucleus) == 2);
  }

  SECTION("epos mass") {
    CHECK_FALSE(corsika::epos::getEposMass(Code::Electron) / 1_GeV == Approx(0));
  }

  /*

    This part does belong to validation rather than the interface tests

   */
  SECTION("validation - pdg id") {
    for (auto p : get_all_particles()) {
      if (!is_nucleus(p)) {
        int eid = corsika::epos::convertToEposRaw(p);
        if (eid == 0 && p != Code::Unknown)
          CHECK_FALSE(p == convert_from_PDG(getEposPDGId(p)));
        else
          CHECK(p == convert_from_PDG(getEposPDGId(p)));
      }
    }
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

auto sqs2elab(HEPEnergyType const sqs, HEPEnergyType const ma, HEPEnergyType const mb) {
  return (sqs * sqs - ma * ma - mb * mb) / 2. / mb;
}

TEST_CASE("EposInterface", "modules") {

  logging::set_level(logging::level::trace);

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;

  RNGManager<>::getInstance().registerRandomStream("epos");

  SECTION("InteractionInterface - random number") {
    auto const rndm = ::epos::rangen_();
    CHECK(rndm > 0);
    CHECK(rndm < 1);
  }

  SECTION("InteractionInterface - valid targets") {

    Interaction model;
    CHECK_FALSE(model.isValidTarget(Code::Electron));
    CHECK(model.isValidTarget(Code::Hydrogen));
    CHECK(model.isValidTarget(Code::Helium));
    CHECK_FALSE(model.isValidTarget(Code::Iron));
    CHECK(model.isValidTarget(Code::Oxygen));

    // hydrogen target == proton target == neutron target
    auto const [xs_prod_pp, xs_ela_pp] =
        model.getCrossSectionLab(Code::Proton, 1, 1, Code::Proton, 1, 1, 100_GeV);
    auto const [xs_prod_pn, xs_ela_pn] =
        model.getCrossSectionLab(Code::Proton, 1, 1, Code::Neutron, 1, 0, 100_GeV);
    auto const [xs_prod_pHydrogen, xs_ela_pHydrogen] =
        model.getCrossSectionLab(Code::Proton, 1, 1, Code::Hydrogen, 1, 1, 100_GeV);
    CHECK(xs_prod_pp == xs_prod_pHydrogen);
    CHECK(xs_prod_pp == xs_prod_pn);
    CHECK(xs_ela_pp == xs_ela_pHydrogen);
    CHECK(xs_ela_pn == xs_ela_pHydrogen);
  }

  SECTION("InteractionInterface - hadron cross sections") {

    Interaction model;

    // p-p at 7TeV around 70mb according to LHC
    auto const [xs_prod, xs_ela] =
        model.getCrossSectionLab(Code::Proton, 1, 1, Code::Proton, 1, 1,
                                 sqs2elab(7_TeV, Proton::mass, Proton::mass));
    CHECK(xs_prod / 1_mb == Approx(70.7).margin(2.1));
    { [[maybe_unused]] auto const& dum_xs = xs_ela; }

    // pi-n at 7TeV
    auto const [xs_prod1, xs_ela1] =
        model.getCrossSectionLab(Code::PiPlus, 0, 0, Code::Neutron, 1, 0,
                                 sqs2elab(7_TeV, PiPlus::mass, Neutron::mass));
    CHECK(xs_prod1 / 1_mb == Approx(52.7).margin(2.1));
    { [[maybe_unused]] auto const& dum_xs = xs_ela1; }

    // k-p at 7TeV
    auto const [xs_prod2, xs_ela2] =
        model.getCrossSectionLab(Code::KPlus, 0, 0, Code::Proton, 1, 1,
                                 sqs2elab(7_TeV, KPlus::mass, Proton::mass));
    CHECK(xs_prod2 / 1_mb == Approx(45.7).margin(2.1));
    { [[maybe_unused]] auto const& dum_xs = xs_ela2; }
  }

  SECTION("InteractionInterface - nuclear cross sections") {

    Interaction model;

    auto const [xs_prod, xs_ela] = model.getCrossSectionLab(
        Code::Proton, 1, 1, Code::Oxygen, Oxygen::nucleus_A, Oxygen::nucleus_Z, 100_GeV);
    CHECK(xs_prod / 1_mb == Approx(287.0).margin(5.1));
    { [[maybe_unused]] auto const& dum_xs = xs_ela; }

    auto const [xs_prod2, xs_ela2] = model.getCrossSectionLab(
        Code::Nitrogen, Nitrogen::nucleus_A, Nitrogen::nucleus_Z, Code::Oxygen,
        Oxygen::nucleus_A, Oxygen::nucleus_Z, 400_GeV);
    CHECK(xs_prod2 / 1_mb == Approx(1076.7).margin(3.1));
    { [[maybe_unused]] auto const& dum_xs = xs_ela2; }

    // nuclear stack extension, particle "Nucleus"
    auto const [xs_prod3, xs_ela3] = model.getCrossSectionLab(
        Code::Nucleus, Nitrogen::nucleus_A, Nitrogen::nucleus_Z, Code::Oxygen,
        Oxygen::nucleus_A, Oxygen::nucleus_Z, 400_GeV);
    CHECK(xs_prod2 / xs_prod3 == 1);
    { [[maybe_unused]] auto const& dum_xs = xs_ela3; }
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

    auto const pSum = sumMomentum(view, cs);

    // this is not physics validation
    CHECK(pSum.getComponents(cs).getX() / P0 == Approx(1).margin(0.05));
    CHECK(pSum.getComponents(cs).getY() / 1_GeV == Approx(0).margin(.5));
    CHECK(pSum.getComponents(cs).getZ() / 1_GeV == Approx(0).margin(.5));

    CHECK((pSum - plab).getNorm() / 1_GeV ==
          Approx(0).margin(plab.getNorm() * 0.05 / 1_GeV));
    CHECK(pSum.getNorm() / P0 == Approx(1).margin(0.05));

    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);
    CHECK(length / 1_g * 1_cm * 1_cm == Approx(93.3).margin(0.1));
  }

  SECTION("InteractionInterface - nuclear projectile") {

    const HEPEnergyType P0 = 10_TeV;
    auto [stack, viewPtr] = setup::testing::setup_stack(
        Code::Nucleus, 8, 4, P0, (setup::Environment::BaseNodeType* const)nodePtr, cs);
    MomentumVector plab =
        MomentumVector(cs, {P0, 0_eV, 0_eV}); // this is secret knowledge about setupStack
    setup::StackView& view = *viewPtr;

    auto particle = stack->first();

    Interaction model;

    /*
    #ifndef __clang__
        // This is very obscure since it fails for -O2, but for both clang and gcc ???
        model.doInteraction(view);

        auto const pSum = sumMomentum(view, cs);

        CHECK(pSum.getComponents(cs).getX() / P0 == Approx(1).margin(0.05));
        CHECK(pSum.getComponents(cs).getY() / 1_GeV == Approx(0).margin(1e-4));
        CHECK(pSum.getComponents(cs).getZ() / 1_GeV == Approx(0).margin(1e-4));

        CHECK((pSum - plab).getNorm() / 1_GeV ==
              Approx(0).margin(plab.getNorm() * 0.05 / 1_GeV));
        CHECK(pSum.getNorm() / P0 == Approx(1).margin(0.05));
    #endif
    */
    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);
    CHECK(length / 1_g * 1_cm * 1_cm ==
          Approx(30).margin(20)); // this is no physics validation
  }
}
