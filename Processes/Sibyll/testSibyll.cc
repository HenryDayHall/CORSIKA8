
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/sibyll/Decay.h>
#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/ParticleConversion.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::sibyll;

TEST_CASE("Sibyll", "[processes]") {

  SECTION("Sibyll -> Corsika") {
    REQUIRE(corsika::particles::Electron::GetCode() ==
            process::sibyll::ConvertFromSibyll(process::sibyll::SibyllCode::Electron));
  }

  SECTION("Corsika -> Sibyll") {
    REQUIRE(process::sibyll::ConvertToSibyll(corsika::particles::Electron::GetCode()) ==
            process::sibyll::SibyllCode::Electron);
    REQUIRE(process::sibyll::ConvertToSibyllRaw(corsika::particles::Proton::GetCode()) ==
            13);
  }

  SECTION("KnownBySibyll") {
    REQUIRE(process::sibyll::KnownBySibyll(corsika::particles::Electron::GetCode()));

    REQUIRE_FALSE(
        process::sibyll::KnownBySibyll(corsika::particles::XiPrimeC0::GetCode()));
  }

  SECTION("canInteractInSibyll") {

    REQUIRE(process::sibyll::CanInteract(corsika::particles::Proton::GetCode()));
    REQUIRE(process::sibyll::CanInteract(corsika::particles::Code::XiCPlus));

    REQUIRE_FALSE(process::sibyll::CanInteract(corsika::particles::Electron::GetCode()));
    REQUIRE_FALSE(process::sibyll::CanInteract(corsika::particles::SigmaC0::GetCode()));
  }

  SECTION("cross-section type") {

    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::Electron) == 0);
    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::K0Long) == 3);
    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::SigmaPlus) == 1);
    REQUIRE(process::sibyll::GetSibyllXSCode(corsika::particles::Code::PiMinus) == 2);
  }
}

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/particles/ParticleProperties.h>

using namespace corsika::units::si;
using namespace corsika::units;

TEST_CASE("SibyllInterface", "[processes]") {

    // setup environment, geometry
  corsika::environment::Environment env;
  auto& universe = *(env.GetUniverse());

  auto theMedium = corsika::environment::Environment::CreateNode<geometry::Sphere>(
									 geometry::Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel =
      corsika::environment::HomogeneousMedium<corsika::environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      corsika::environment::NuclearComposition(
          std::vector<corsika::particles::Code>{corsika::particles::Code::Oxygen},
          std::vector<float>{1.}));

  universe.AddChild(std::move(theMedium));

  const geometry::CoordinateSystem& cs = env.GetCoordinateSystem();

  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  geometry::Vector<corsika::units::si::SpeedType::dimension_type> v(
      cs, 0_m / second, 0_m / second, 1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  
  SECTION("InteractionInterface") {

    setup::Stack stack;
    auto particle = stack.NewParticle();
    
    Interaction model(env);
    
    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoInteraction(particle, stack);
    [[maybe_unused]] const GrammageType length =
        model.GetInteractionLength(particle, track);
  }

  SECTION("DecayInterface") {

    setup::Stack stack;
    auto particle = stack.NewParticle();
    {
      const HEPEnergyType E0 = 10_GeV;
      particle.SetPID(particles::Code::Proton);
      HEPMomentumType P0 = sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
      auto plab = stack::super_stupid::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
      particle.SetEnergy(E0);
      particle.SetMomentum(plab);
      particle.SetTime(0_ns);
      geometry::Point p(cs, 0_m, 0_m, 0_m);
      particle.SetPosition(p);
    }
    
    Decay model;
    
    model.Init();
    /*[[maybe_unused]] const process::EProcessReturn ret =*/model.DoDecay(particle,
                                                                          stack);
    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
  }
}
