/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/qgsjetII/Interaction.hpp>
#include <corsika/modules/qgsjetII/ParticleConversion.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::qgsjetII;

TEST_CASE("QgsjetII", "[processes]") {

  SECTION("QgsjetII -> Corsika") {
    REQUIRE(corsika::PiPlus::GetCode() == corsika::qgsjetII::ConvertFromQgsjetII(
                                              corsika::qgsjetII::QgsjetIICode::PiPlus));
  }

  SECTION("Corsika -> QgsjetII") {
    REQUIRE(corsika::qgsjetII::ConvertToQgsjetII(corsika::PiMinus::GetCode()) ==
            corsika::qgsjetII::QgsjetIICode::PiMinus);
    REQUIRE(corsika::qgsjetII::ConvertToQgsjetIIRaw(corsika::Proton::GetCode()) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    REQUIRE(corsika::qgsjetII::CanInteract(corsika::Proton::GetCode()));
    REQUIRE(corsika::qgsjetII::CanInteract(corsika::Code::KPlus));
    REQUIRE(corsika::qgsjetII::CanInteract(corsika::Nucleus::GetCode()));
    // REQUIRE(corsika::qgsjetII::CanInteract(corsika::Helium::GetCode()));

    REQUIRE_FALSE(corsika::qgsjetII::CanInteract(corsika::EtaC::GetCode()));
    REQUIRE_FALSE(corsika::qgsjetII::CanInteract(corsika::SigmaC0::GetCode()));
  }

  SECTION("cross-section type") {

    REQUIRE(corsika::qgsjetII::GetQgsjetIIXSCode(corsika::Code::Neutron) == 2);
    REQUIRE(corsika::qgsjetII::GetQgsjetIIXSCode(corsika::Code::K0Long) == 3);
    REQUIRE(corsika::qgsjetII::GetQgsjetIIXSCode(corsika::Code::Proton) == 2);
    REQUIRE(corsika::qgsjetII::GetQgsjetIIXSCode(corsika::Code::PiMinus) == 1);
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

TEST_CASE("QgsjetIIInterface", "[processes]") {

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
      corsika::NuclearComposition(std::vector<corsika::Code>{corsika::Code::Oxygen},
                                  std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  const corsika::CoordinateSystem& cs = env.GetCoordinateSystem();

  corsika::RNGManager::GetInstance().RegisterRandomStream("qgran");

  SECTION("InteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - corsika::Proton::GetMass() * corsika::Proton::GetMass());
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    corsika::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector, corsika::Point,
                   TimeType, unsigned int, unsigned int>{corsika::Code::Nucleus, E0, plab,
                                                         pos, 0_ns, 16, 8});

    particle.SetNode(nodePtr);
    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;
    model.Init();
    [[maybe_unused]] const corsika::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }
}
