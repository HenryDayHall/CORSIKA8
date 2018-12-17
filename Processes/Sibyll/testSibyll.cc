
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

using namespace corsika::units::si;

TEST_CASE("SibyllInterface", "[processes]") {

  auto const& cs =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  geometry::Vector<corsika::units::si::SpeedType::dimension_type> v(
      cs, 0_m / second, 0_m / second, 1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  setup::Stack stack;
  auto particle = stack.NewParticle();

  SECTION("InteractionInterface") {

    Interaction model;

    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret =
        model.DoInteraction(particle, stack);
    [[maybe_unused]] const double length = model.GetInteractionLength(particle, track);
  }

  SECTION("DecayInterface") {

    Decay model;

    model.Init();
    /*[[maybe_unused]] const process::EProcessReturn ret =*/model.DoDecay(particle,
                                                                          stack);
    [[maybe_unused]] const double length = model.GetLifetime(particle);
  }
}
