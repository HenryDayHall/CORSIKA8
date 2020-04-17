/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/QGSJetII.hpp>

#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

#include <cstdlib>
#include <experimental/filesystem>
#include <iostream>

using namespace corsika;
using namespace corsika::qgsjetII;

TEST_CASE("QgsjetII", "[processes]") {

  SECTION("Corsika -> QgsjetII") {
    CHECK(process::qgsjetII::ConvertToQgsjetII(particles::PiMinus::GetCode()) ==
          process::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(process::qgsjetII::ConvertToQgsjetIIRaw(particles::Proton::GetCode()) == 2);
  }

  SECTION("QgsjetII -> Corsika") {
    CHECK(particles::PiPlus::GetCode() == process::qgsjetII::ConvertFromQgsjetII(
                                              process::qgsjetII::QgsjetIICode::PiPlus));
  }

  SECTION("Corsika -> QgsjetII") {
    CHECK(process::qgsjetII::ConvertToQgsjetII(particles::PiMinus::GetCode()) ==
          process::qgsjetII::QgsjetIICode::PiMinus);
    CHECK(process::qgsjetII::ConvertToQgsjetIIRaw(particles::Proton::GetCode()) == 2);
  }

  SECTION("canInteractInQgsjetII") {

    CHECK(process::qgsjetII::CanInteract(particles::Proton::GetCode()));
    CHECK(process::qgsjetII::CanInteract(particles::Code::KPlus));
    CHECK(process::qgsjetII::CanInteract(particles::Nucleus::GetCode()));
    // CHECK(process::qgsjetII::CanInteract(particles::Helium::GetCode()));

    CHECK_FALSE(process::qgsjetII::CanInteract(particles::EtaC::GetCode()));
    CHECK_FALSE(process::qgsjetII::CanInteract(particles::SigmaC0::GetCode()));
  }

  SECTION("cross-section type") {

    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::Neutron) ==
          process::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::K0Long) ==
          process::qgsjetII::QgsjetIIXSClass::Kaons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::Proton) ==
          process::qgsjetII::QgsjetIIXSClass::Baryons);
    CHECK(process::qgsjetII::GetQgsjetIIXSCode(particles::Code::PiMinus) ==
          process::qgsjetII::QgsjetIIXSClass::LightMesons);
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

using namespace corsika::units::si;
using namespace corsika::units;
using namespace corsika;

TEST_CASE("QgsjetIIInterface", "[processes]") {

  auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  corsika::random::RNGManager::GetInstance().RegisterRandomStream("qgsjet");

  SECTION("InteractionInterface") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::Proton::GetMass() * particles::Proton::GetMass());
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle =
        stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                     corsika::MomentumVector, geometry::Point,
                                     units::si::TimeType, unsigned int, unsigned int>{
            particles::Code::Nucleus, E0, plab, pos, 0_ns, 16, 8});
    // corsika::MomentumVector, geometry::Point, units::si::TimeType>{
    //	  particles::Code::PiPlus, E0, plab, pos, 0_ns});

    particle.SetNode(nodePtr);
    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    Interaction model;

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(view);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);

    CHECK(length / (1_g / square(1_cm)) == Approx(93.04).margin(0.1));

    /***********************************
     It as turned out already two times (#291 and #307) that the detailed output of
    QGSJetII event generation depends on the gfortran version used. This is not reliable
    and cannot be tested in a unit test here. One related problem was already found (#291)
    and is realted to undefined behaviour in the evaluation of functions in logical
    expressions. It is not clear if #307 is the same issue.

     CHECK(view.GetSize() == 14);
     CHECK(sumCharge(view) == 2);
    ************************************/
    auto const secMomSum = sumMomentum(view, projectileMomentum.GetCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
          Approx(0).margin(1e-2));
  }
}
