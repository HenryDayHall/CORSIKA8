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
    REQUIRE(Code::Electron ==
            corsika::sibyll::ConvertFromSibyll(corsika::sibyll::SibyllCode::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    REQUIRE(corsika::sibyll::ConvertToSibyll(Code::Electron) ==
            corsika::sibyll::SibyllCode::Electron);
    REQUIRE(corsika::sibyll::ConvertToSibyllRaw(Code::Proton) == 13);
  }

  SECTION("canInteractInSibyll") {

    REQUIRE(corsika::sibyll::CanInteract(Code::Proton));
    REQUIRE(corsika::sibyll::CanInteract(Code::XiCPlus));

    REQUIRE_FALSE(corsika::sibyll::CanInteract(Code::Electron));
    REQUIRE_FALSE(corsika::sibyll::CanInteract(Code::SigmaC0));

    REQUIRE_FALSE(corsika::sibyll::CanInteract(Code::Nucleus));
    REQUIRE_FALSE(corsika::sibyll::CanInteract(Code::Helium));
  }

  SECTION("cross-section type") {

    REQUIRE(corsika::sibyll::GetSibyllXSCode(Code::Electron) == 0);
    REQUIRE(corsika::sibyll::GetSibyllXSCode(Code::K0Long) == 3);
    REQUIRE(corsika::sibyll::GetSibyllXSCode(Code::SigmaPlus) == 1);
    REQUIRE(corsika::sibyll::GetSibyllXSCode(Code::PiMinus) == 2);
  }
}

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <sibyll2.3d.hpp>

using namespace corsika::units::si;
using namespace corsika::units;

TEST_CASE("SibyllInterface", "[processes]") {

  // setup environment, geometry
  corsika::Environment<corsika::IMediumModel> env;
  auto& universe = *(env.GetUniverse());

  auto theMedium =
      corsika::Environment<corsika::IMediumModel>::CreateNode<corsika::Sphere>(
          corsika::Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = corsika::HomogeneousMedium<corsika::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      corsika::NuclearComposition(std::vector<Code>{Code::Oxygen},
                                  std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  const corsika::CoordinateSystem& cs = env.GetCoordinateSystem();

  corsika::RNGManager::getInstance().registerRandomStream("sibyll");

  SECTION("InteractionInterface") {

    corsika::setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 = sqrt(E0 * E0 - Proton::mass * Proton::mass);
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    corsika::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<Code, corsika::units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, corsika::units::si::TimeType>{Code::Proton, E0, plab,
                                                                 pos, 0_ns});
    particle.SetNode(nodePtr);
    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;

    model.Init();
    [[maybe_unused]] const corsika::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }

  SECTION("NuclearInteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 400_GeV;
    HEPMomentumType P0 = sqrt(E0 * E0 - Proton::mass * Proton::mass);
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    corsika::Point pos(cs, 0_m, 0_m, 0_m);

    auto particle = stack.AddParticle(
        std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType, unsigned short, unsigned short>{
            Code::Nucleus, E0, plab, pos, 0_ns, 4, 2});
    particle.SetNode(nodePtr);
    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction hmodel;
    NuclearInteraction model(hmodel, env);

    model.Init();
    [[maybe_unused]] const corsika::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }

  SECTION("DecayInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 10_GeV;
    HEPMomentumType P0 = sqrt(E0 * E0 - Proton::mass * Proton::mass);
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    corsika::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{Code::Lambda0, E0, plab, pos,
                                                        0_ns});
    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Decay model;

    model.Init();
    /*[[maybe_unused]] const corsika::EProcessReturn ret =*/model.DoDecay(projectile);
    // run checks
    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
  }

  SECTION("DecayConfiguration") {

    Decay model;

    const std::vector<Code> particleTestList = {Code::PiPlus, Code::PiMinus,
                                                Code::KPlus,  Code::Lambda0Bar,
                                                Code::NuE,    Code::D0Bar};

    for (auto& pCode : particleTestList) {
      model.SetUnstable(pCode);
      // get state of sibyll internal config
      REQUIRE(0 <= s_csydec_.idb[abs(corsika::sibyll::ConvertToSibyllRaw(pCode)) - 1]);

      model.SetStable(pCode);
      // get state of sibyll internal config
      REQUIRE(0 >= s_csydec_.idb[abs(corsika::sibyll::ConvertToSibyllRaw(pCode)) - 1]);
    }
  }
}
