/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/Sibyll.hpp>
#include <corsika/modules/sibyll/ParticleConversion.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <catch2/catch.hpp>
#include <tuple>

using namespace corsika;
using namespace corsika::sibyll;

TEST_CASE("Sibyll", "[processes]") {

  SECTION("Sibyll -> Corsika") {
    CHECK(Code::Electron ==
          corsika::sibyll::convertFromSibyll(corsika::sibyll::SibyllCode::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    CHECK(corsika::sibyll::convertToSibyll(Electron::code) ==
          corsika::sibyll::SibyllCode::Electron);
    CHECK(corsika::sibyll::convertToSibyllRaw(Proton::code) == 13);
    CHECK(corsika::sibyll::convertToSibyll(XiStarC0::code) ==
          corsika::sibyll::SibyllCode::XiStarC0);
  }

  SECTION("canInteractInSibyll") {

    CHECK(corsika::sibyll::canInteract(Code::Proton));
    CHECK(corsika::sibyll::canInteract(Code::XiCPlus));

    CHECK_FALSE(corsika::sibyll::canInteract(Code::Electron));
    CHECK_FALSE(corsika::sibyll::canInteract(Code::SigmaC0));

    CHECK_FALSE(corsika::sibyll::canInteract(Code::Nucleus));
    CHECK_FALSE(corsika::sibyll::canInteract(Code::Helium));
  }

  SECTION("cross-section type") {

    CHECK(corsika::sibyll::getSibyllXSCode(Code::Electron) == 0);
    CHECK(corsika::sibyll::getSibyllXSCode(Code::K0Long) == 3);
    CHECK(corsika::sibyll::getSibyllXSCode(Code::SigmaPlus) == 1);
    CHECK(corsika::sibyll::getSibyllXSCode(Code::PiMinus) == 2);
  }

  SECTION("sibyll mass") {

    CHECK_FALSE(corsika::sibyll::getSibyllMass(Code::Electron) == 0_GeV);
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

TEST_CASE("SibyllInterface", "[processes]") {

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;

  RNGManager::getInstance().registerRandomStream("sibyll");

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

    /*
      Interactions between hadrons (h) and nuclei (A) in Sibyll are treated in the
      hadron-nucleon center-of-mass frame (hnCoM). The incoming hadron (h) and
      nucleon (N) are assumed massless, such that the energy and momentum in the hnCoM are
      : E_i_cm = 0.5 * SQS and P_i_cm = +- 0.5 * SQS  where i is either the projectile
      hadron or the target nucleon and SQS is the hadron-nucleon center-of-mass energy.

      The true energies and momenta, accounting for the hadron masses, are: E_i = ( S +
      m_i**2 - m_j**2 ) / (2 * SQS) and Pcm = +-
      sqrt( (S-(m_j+m_i)**2) * (s-(m_j-m_i)**2) ) / (2*SQS) where m_i is the projectiles
      mass and m_j is the target particles mass. In terms of lab. frame variables Pcm =
      m_j * Plab_i / SQS, where Plab_i is the momentum of the projectile (i) in the lab.
      and m_j is the mass of the target, i.e. the particle at rest (usually a nucleon).

      Any hadron-nucleus event can contain several nucleon interactions. In case of Nw
      (number of wounded nucleons) nucleons interacting in the hadron-nucleus interaction,
      the total energy and momentum in the hadron(i)-nucleon(N) center-of-mass frame are:
      momentum: p_projectile + p_nucleon_1 + p_nucleon_2 + .... p_nucleon_Nw = -(Nw-1) *
      Pcm with center-of-mass momentum Pcm = p_projectile = - p_nucleon_i. For the energy:
      E_projectile + E_nucleon_1 + ... E_nucleon_Nw = E_projectile + Nw * E_nucleon.

      Using the above definitions of center-of-mass energies and momenta this leads to the
      total energy: E_tot = SQS/2 * (1+Nw) + (m_N**2-m_i**2)/(2*SQS) * (Nw-1) and P_tot
      = -m_N * Plab_i / SQS * (Nw-1).

      A Lorentztransformation of these quantities to the lab. frame recovers Plab_i for
      the total momentum, so momentum is exactly conserved, and Elab_i + Nw * m_N for the
      total energy. Not surprisingly the total energy differs from the total energy before
      the collision by the mass of the additional nucleons (Nw-1)*m_N. In relative terms
      the additional energy is entirely negligible and as it is not kinetic energy there
      is zero influence on the shower development.

      Due to the ommission of the hadron masses in Sibyll, the total energy and momentum
      in the center-of-mass system after the collision are just: E_tot = SQS/2 * (1+Nw)
      and P_tot = SQS/2 * (1-Nw). After the Lorentztransformation the total momentum in
      the lab. thus differs from the initial value by (1-Nw)/2 * ( m_N + m_i**2 / (2 *
      Plab_i) ) and momentum is NOT conserved. Note however that the second term quickly
      vanishes as the lab. momentum of the projectile increases. The first term is fixed
      as it depends only on the number of additional nucleons, in relative terms it is
      always small at high energies.

      For this reason the numerical precision in these tests is limited to 5% to still
      pass at low energies and no absolute check is implemented, e.g.

          CHECK(pSum.getComponents(cs).getX() / P0 == Approx(1).margin(0.05));
          CHECK((pSum - plab).norm()/1_GeV == Approx(0).margin(plab.norm() * 0.05/1_GeV));

      /FR'2020

      See also:

      Issue 272 / MR 204
      https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/merge_requests/204

    */

    CHECK(pSum.getComponents(cs).getX() / P0 == Approx(1).margin(0.05));
    CHECK(pSum.getComponents(cs).getY() / 1_GeV == Approx(0).margin(1e-4));
    CHECK(pSum.getComponents(cs).getZ() / 1_GeV == Approx(0).margin(1e-4));

    CHECK((pSum - plab).getNorm() / 1_GeV ==
          Approx(0).margin(plab.getNorm() * 0.05 / 1_GeV));
    CHECK(pSum.getNorm() / P0 == Approx(1).margin(0.05));
    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);
    CHECK(length / 1_g * 1_cm * 1_cm == Approx(88.7).margin(0.1));
    CHECK(view.getEntries() == 9); //! \todo: this was 20 before refactory-2020: check
  }

  SECTION("NuclearInteractionInterface") {

    auto [stack, viewPtr] =
        setup::testing::setup_stack(Code::Nucleus, 4, 2, 500_GeV,
                                    (setup::Environment::BaseNodeType* const)nodePtr, cs);
    setup::StackView& view = *viewPtr;
    auto particle = stack->first();

    Interaction hmodel;
    NuclearInteraction model(hmodel, *env);

    model.doInteraction(view);
    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);
    // Felix, are those changes OK? Below are the checks before refactory-2020
    // CHECK(length / 1_g * 1_cm * 1_cm == Approx(44.2).margin(.1));
    // CHECK(view.getSize() == 11);
    CHECK(length / 1_g * 1_cm * 1_cm == Approx(42.8).margin(.1));
    CHECK(view.getSize() == 40);
  }

  SECTION("DecayInterface") {

    auto [stackPtr, viewPtr] =
        setup::testing::setup_stack(Code::Lambda0, 0, 0, 10_GeV,
                                    (setup::Environment::BaseNodeType* const)nodePtr, cs);
    setup::StackView& view = *viewPtr;
    auto& stack = *stackPtr;
    auto particle = stack.first();

    Decay model;
    model.printDecayConfig();
    [[maybe_unused]] const TimeType time = model.getLifetime(particle);

    model.doDecay(view);
    // run checks
    // lambda decays into proton and pi- or neutron and pi+
    CHECK(stack.getEntries() == 3);
  }

  SECTION("DecayConfiguration") {

    Decay model({Code::PiPlus, Code::PiMinus});
    CHECK(model.isDecayHandled(Code::PiPlus));
    CHECK(model.isDecayHandled(Code::PiMinus));
    CHECK_FALSE(model.isDecayHandled(Code::KPlus));

    const std::vector<Code> particleTestList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                                Code::Lambda0Bar, Code::D0Bar};

    // setup decays
    model.setHandleDecay(particleTestList);
    for (auto& pCode : particleTestList) CHECK(model.isDecayHandled(pCode));

    // individually
    model.setHandleDecay(Code::KMinus);

    // possible decays
    CHECK_FALSE(model.canHandleDecay(Code::Proton));
    CHECK_FALSE(model.canHandleDecay(Code::Electron));
    CHECK(model.canHandleDecay(Code::PiPlus));
    CHECK(model.canHandleDecay(Code::MuPlus));
  }
}
