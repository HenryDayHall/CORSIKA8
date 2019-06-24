/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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

TEST_CASE("Sibyll", "[processes]") {

  SECTION("Sibyll -> Corsika") {
    REQUIRE(particles::Electron::GetCode() ==
            process::sibyll::ConvertFromSibyll(process::sibyll::SibyllCode::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    REQUIRE(process::sibyll::ConvertToSibyll(particles::Electron::GetCode()) ==
            process::sibyll::SibyllCode::Electron);
    REQUIRE(process::sibyll::ConvertToSibyllRaw(particles::Proton::GetCode()) == 13);
  }

  SECTION("canInteractInSibyll") {

    REQUIRE(process::sibyll::CanInteract(particles::Proton::GetCode()));
    REQUIRE(process::sibyll::CanInteract(particles::Code::XiCPlus));

    REQUIRE_FALSE(process::sibyll::CanInteract(particles::Electron::GetCode()));
    REQUIRE_FALSE(process::sibyll::CanInteract(particles::SigmaC0::GetCode()));

    REQUIRE_FALSE(process::sibyll::CanInteract(particles::Nucleus::GetCode()));
    REQUIRE_FALSE(process::sibyll::CanInteract(particles::Helium::GetCode()));
  }

  SECTION("cross-section type") {

    REQUIRE(process::sibyll::GetSibyllXSCode(particles::Code::Electron) == 0);
    REQUIRE(process::sibyll::GetSibyllXSCode(particles::Code::K0Long) == 3);
    REQUIRE(process::sibyll::GetSibyllXSCode(particles::Code::SigmaPlus) == 1);
    REQUIRE(process::sibyll::GetSibyllXSCode(particles::Code::PiMinus) == 2);
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
#include <corsika/process/sibyll/sibyll2.3c.h>

using namespace corsika::units::si;
using namespace corsika::units;

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

  random::RNGManager::GetInstance().RegisterRandomStream("s_rndm");

  SECTION("InteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Proton, E0, plab, pos, 0_ns});
    particle.SetNode(nodePtr);
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;

    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
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

    model.Init();
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

    model.Init();
    /*[[maybe_unused]] const process::EProcessReturn ret =*/model.DoDecay(projectile);
    // run checks
    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
  }

  SECTION("DecayConfiguration") {

    Decay model;

    const std::vector<particles::Code> particleTestList = {
        particles::Code::PiPlus,     particles::Code::PiMinus, particles::Code::KPlus,
        particles::Code::Lambda0Bar, particles::Code::NuE,     particles::Code::D0Bar};

    for (auto& pCode : particleTestList) {
      model.SetUnstable(pCode);
      // get state of sibyll internal config
      REQUIRE(0 <= s_csydec_.idb[abs(process::sibyll::ConvertToSibyllRaw(pCode)) - 1]);

      model.SetStable(pCode);
      // get state of sibyll internal config
      REQUIRE(0 >= s_csydec_.idb[abs(process::sibyll::ConvertToSibyllRaw(pCode)) - 1]);
    }
  }
}
