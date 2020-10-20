/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <Pythia8/Pythia.h>
#include <corsika/process/pythia/Decay.h>
#include <corsika/process/pythia/Interaction.h>

#include <corsika/random/RNGManager.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <catch2/catch.hpp>

TEST_CASE("Pythia", "[processes]") {

  SECTION("linking pythia") {
    using namespace Pythia8;
    using std::cout;
    using std::endl;

    // Generator. Process selection. LHC initialization. Histogram.
    Pythia pythia;

    pythia.readString("Next:numberShowInfo = 0");
    pythia.readString("Next:numberShowProcess = 0");
    pythia.readString("Next:numberShowEvent = 0");

    pythia.readString("ProcessLevel:all = off");

    pythia.init();

    Event& event = pythia.event;
    event.reset();

    pythia.particleData.mayDecay(321, true);
    double pz = 100.;
    double m = 0.49368;
    event.append(321, 1, 0, 0, 0., 0., 100., sqrt(pz * pz + m * m), m);

    if (!pythia.next())
      cout << "decay failed!" << endl;
    else
      cout << "particles after decay: " << event.size() << endl;
    event.list();

    // loop over final state
    for (int i = 0; i < pythia.event.size(); ++i)
      if (pythia.event[i].isFinal()) {
        cout << "particle: id=" << pythia.event[i].id() << endl;
      }
  }

  SECTION("pythia interface") {
    using namespace corsika;

    const std::vector<particles::Code> particleList = {
        particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
        particles::Code::KMinus, particles::Code::K0Long,  particles::Code::K0Short};

    random::RNGManager::GetInstance().RegisterRandomStream("pythia");

    process::pythia::Decay model(particleList);

    model.Init();
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

using namespace corsika;
using namespace corsika::units::si;

TEST_CASE("pythia process") {

  // setup environment, geometry
  environment::Environment<environment::IMediumModel> env;

  geometry::CoordinateSystem const& cs = env.GetCoordinateSystem();

  auto theMedium =
      environment::Environment<environment::IMediumModel>::CreateNode<geometry::Sphere>(
          geometry::Point{cs, 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Hydrogen},
          std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get(); // save the medium for later use before moving it

  SECTION("pythia decay") {

    setup::Stack stack;
    const HEPEnergyType E0 = 10_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::PiPlus::GetMass() * particles::PiPlus::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::PiPlus, E0, plab, pos, 0_ns});

    const std::vector<particles::Code> particleList = {
        particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
        particles::Code::KMinus, particles::Code::K0Long,  particles::Code::K0Short};

    random::RNGManager::GetInstance().RegisterRandomStream("pythia");

    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    process::pythia::Decay model(particleList);
    model.Init();
    model.DoDecay(projectile);
    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
  }

  SECTION("pythia interaction") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::PiPlus::GetMass() * particles::PiPlus::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::PiPlus, E0, plab, pos, 0_ns});
    particle.SetNode(nodePtr);
    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    process::pythia::Interaction model;
    model.Init();
    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }
}
