/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>
#include <corsika/process/sibyll/ParticleConversion.h>

#include <corsika/random/RNGManager.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::sibyll;
using namespace corsika::units;
using namespace corsika::units::si;

TEST_CASE("Sibyll", "[processes]") {

  SECTION("Sibyll -> Corsika") {
    CHECK(particles::Electron::GetCode() ==
          process::sibyll::ConvertFromSibyll(process::sibyll::SibyllCode::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    CHECK(process::sibyll::ConvertToSibyll(particles::Electron::GetCode()) ==
          process::sibyll::SibyllCode::Electron);
    CHECK(process::sibyll::ConvertToSibyllRaw(particles::Proton::GetCode()) == 13);
    CHECK(process::sibyll::ConvertToSibyll(particles::XiStarC0::GetCode()) ==
          process::sibyll::SibyllCode::XiStarC0);
  }

  SECTION("canInteractInSibyll") {

    CHECK(process::sibyll::CanInteract(particles::Proton::GetCode()));
    CHECK(process::sibyll::CanInteract(particles::Code::XiCPlus));

    CHECK_FALSE(process::sibyll::CanInteract(particles::Electron::GetCode()));
    CHECK_FALSE(process::sibyll::CanInteract(particles::SigmaC0::GetCode()));

    CHECK_FALSE(process::sibyll::CanInteract(particles::Nucleus::GetCode()));
    CHECK_FALSE(process::sibyll::CanInteract(particles::Helium::GetCode()));
  }

  SECTION("cross-section type") {

    CHECK(process::sibyll::GetSibyllXSCode(particles::Code::Electron) == 0);
    CHECK(process::sibyll::GetSibyllXSCode(particles::Code::K0Long) == 3);
    CHECK(process::sibyll::GetSibyllXSCode(particles::Code::SigmaPlus) == 1);
    CHECK(process::sibyll::GetSibyllXSCode(particles::Code::PiMinus) == 2);
  }

  SECTION("sibyll mass") {

    CHECK_FALSE(process::sibyll::GetSibyllMass(particles::Code::Electron) == 0_GeV);
  }
}

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/sibyll/sibyll2.3d.h>

using namespace corsika::units::si;
using namespace corsika::units;

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}

TEST_CASE("SibyllInterface", "[processes]") {

  // setup environment, geometry
  environment::Environment<environment::IMediumModel> env;
  auto& universe = *(env.GetUniverse());

  auto theMedium =
      environment::Environment<environment::IMediumModel>::CreateNode<geometry::Sphere>(
          geometry::Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Oxygen}, std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  const geometry::CoordinateSystem& cs = env.GetCoordinateSystem();

  random::RNGManager::GetInstance().RegisterRandomStream("sibyll");

  SECTION("InteractionInterface - low energy") {

    setup::Stack stack;
    const HEPEnergyType E0 = 60_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {P0, 0_eV, 0_eV});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, E0, plab, pos, 0_ns});
    particle.SetNode(nodePtr);
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
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

          CHECK(pSum.GetComponents(cs).GetX() / P0 == Approx(1).margin(0.05));
          CHECK((pSum - plab).norm()/1_GeV == Approx(0).margin(plab.norm() * 0.05/1_GeV));

      /FR'2020

      See also:

      Issue 272 / MR 204
      https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/merge_requests/204

    */

    CHECK(pSum.GetComponents(cs).GetX() / P0 == Approx(1).margin(0.05));
    CHECK(pSum.GetComponents(cs).GetY() / 1_GeV == Approx(0).margin(1e-4));
    CHECK(pSum.GetComponents(cs).GetZ() / 1_GeV == Approx(0).margin(1e-4));

    CHECK((pSum - plab).norm() / 1_GeV == Approx(0).margin(plab.norm() * 0.05 / 1_GeV));
    CHECK(pSum.norm() / P0 == Approx(1).margin(0.05));
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }

  SECTION("InteractionInterface - high energy") {

    setup::Stack stack;
    const HEPEnergyType E0 = 60_EeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {P0, 0_eV, 0_eV});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, E0, plab, pos, 0_ns});
    particle.SetNode(nodePtr);
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
    auto const pSum = sumMomentum(view, cs);
    CHECK(pSum.GetComponents(cs).GetX() / P0 == Approx(1).margin(0.001));
    CHECK(pSum.GetComponents(cs).GetY() / 1_GeV == Approx(0).margin(1e-4));
    CHECK(pSum.GetComponents(cs).GetZ() / 1_GeV == Approx(0).margin(1e-4));

    CHECK((pSum - plab).norm() / 1_GeV == Approx(0).margin(plab.norm() * 0.001 / 1_GeV));
    CHECK(pSum.norm() / P0 == Approx(1).margin(0.05));
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }

  SECTION("NuclearInteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 400_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);

    auto particle =
        stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                     corsika::stack::MomentumVector, geometry::Point,
                                     units::si::TimeType, unsigned short, unsigned short>{
            particles::Code::Nucleus, E0, plab, pos, 0_ns, 4, 2});
    particle.SetNode(nodePtr);
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction hmodel;
    NuclearInteraction model(hmodel, env);

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }

  SECTION("DecayInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 10_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Lambda0, E0, plab, pos, 0_ns});
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Decay model;

    model.PrintDecayConfig();

    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);

    /*[[maybe_unused]] const process::EProcessReturn ret =*/model.DoDecay(projectile);

    // run checks
    // lambda decays into proton and pi- or neutron and pi+
    CHECK(stack.getEntries() == 3);
  }

  SECTION("DecayConfiguration") {

    Decay model({particles::Code::PiPlus, particles::Code::PiMinus});
    CHECK(model.IsDecayHandled(particles::Code::PiPlus));
    CHECK(model.IsDecayHandled(particles::Code::PiMinus));
    CHECK_FALSE(model.IsDecayHandled(particles::Code::KPlus));

    const std::vector<particles::Code> particleTestList = {
        particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
        particles::Code::Lambda0Bar, particles::Code::D0Bar};

    // setup decays
    model.SetHandleDecay(particleTestList);
    for (auto& pCode : particleTestList) CHECK(model.IsDecayHandled(pCode));

    // individually
    model.SetHandleDecay(particles::Code::KMinus);

    // possible decays
    CHECK_FALSE(model.CanHandleDecay(particles::Code::Proton));
    CHECK_FALSE(model.CanHandleDecay(particles::Code::Electron));
    CHECK(model.CanHandleDecay(particles::Code::PiPlus));
    CHECK(model.CanHandleDecay(particles::Code::MuPlus));
  }
}
