/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/qgsjetII/Interaction.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>

#include <corsika/random/RNGManager.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::qgsjetII;

TEST_CASE("QgsjetII", "[processes]") {

  SECTION("QgsjetII -> Corsika") {
    REQUIRE(particles::PiPlus::GetCode() == process::qgsjetII::ConvertFromQgsjetII(
                                                process::qgsjetII::QgsjetIICode::PiPlus));
  }

  SECTION("Corsika -> QgsjetII") {
    REQUIRE(process::qgsjetII::ConvertToQgsjetII(particles::PiMinus::GetCode()) ==
            process::qgsjetII::QgsjetIICode::PiMinus);
    REQUIRE(process::qgsjetII::ConvertToQgsjetIIRaw(particles::Proton::GetCode()) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    REQUIRE(process::qgsjetII::CanInteract(particles::Proton::GetCode()));
    REQUIRE(process::qgsjetII::CanInteract(particles::Code::KPlus));
    REQUIRE(process::qgsjetII::CanInteract(particles::Nucleus::GetCode()));
    // REQUIRE(process::qgsjetII::CanInteract(particles::Helium::GetCode()));

    REQUIRE_FALSE(process::qgsjetII::CanInteract(particles::EtaC::GetCode()));
    REQUIRE_FALSE(process::qgsjetII::CanInteract(particles::SigmaC0::GetCode()));
  }

  SECTION("cross-section type") {

    REQUIRE(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::Neutron) == 2);
    REQUIRE(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::K0Long) == 3);
    REQUIRE(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::Proton) == 2);
    REQUIRE(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::PiMinus) == 1);
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
#include <corsika/process/qgsjetII/qgsjet-II-04.h>

using namespace corsika::units::si;
using namespace corsika::units;

TEST_CASE("QgsjetIIInterface", "[processes]") {

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

  random::RNGManager::GetInstance().RegisterRandomStream("qgran");

  SECTION("InteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle =
        stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                     corsika::stack::MomentumVector, geometry::Point,
                                     units::si::TimeType, unsigned int, unsigned int>{
            particles::Code::Nucleus, E0, plab, pos, 0_ns, 16, 8});
    // corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
    //	  particles::Code::PiPlus, E0, plab, pos, 0_ns});

    particle.SetNode(nodePtr);
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;
    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }
}
