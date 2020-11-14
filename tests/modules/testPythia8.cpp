/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/Pythia8.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/random/RNGManager.hpp>

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

    const std::vector<corsika::Code> particleList = {
        corsika::Code::PiPlus, corsika::Code::PiMinus, corsika::Code::KPlus,
        corsika::Code::KMinus, corsika::Code::K0Long,  corsika::Code::K0Short};

    corsika::RNGManager::getInstance().registerRandomStream("pythia");

    corsika::pythia8::Decay model(particleList);

    model.Init();
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

using namespace corsika;
using namespace corsika::units::si;

TEST_CASE("pythia process") {

  // setup environment, geometry
  corsika::Environment<corsika::IMediumModel> env;

  corsika::CoordinateSystem const& cs = env.GetCoordinateSystem();

  auto theMedium =
      corsika::Environment<corsika::IMediumModel>::CreateNode<corsika::Sphere>(
          corsika::Point{cs, 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = corsika::HomogeneousMedium<corsika::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      corsika::NuclearComposition(std::vector<corsika::Code>{corsika::Code::Hydrogen},
                                  std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get(); // save the medium for later use before moving it

  SECTION("pythia decay") {

    setup::Stack stack;
    const HEPEnergyType E0 = 10_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - corsika::PiPlus::GetMass() * corsika::PiPlus::GetMass());
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    corsika::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{corsika::Code::PiPlus, E0, plab,
                                                        pos, 0_ns});

    const std::vector<corsika::Code> particleList = {
        corsika::Code::PiPlus, corsika::Code::PiMinus, corsika::Code::KPlus,
        corsika::Code::KMinus, corsika::Code::K0Long,  corsika::Code::K0Short};

    corsika::RNGManager::getInstance().registerRandomStream("pythia");

    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    corsika::pythia8::Decay model(particleList);
    model.Init();
    model.DoDecay(projectile);
    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
  }

  SECTION("pythia interaction") {

    setup::Stack stack;
    const HEPEnergyType E0 = 100_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - corsika::PiPlus::GetMass() * corsika::PiPlus::GetMass());
    auto plab = corsika::MomentumVector(cs, {0_GeV, 0_GeV, -P0});
    corsika::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                   corsika::Point, units::si::TimeType>{corsika::Code::PiPlus, E0, plab,
                                                        pos, 0_ns});
    particle.SetNode(nodePtr);
    corsika::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    corsika::pythia8::Interaction model;
    model.Init();
    [[maybe_unused]] const corsika::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }
}
