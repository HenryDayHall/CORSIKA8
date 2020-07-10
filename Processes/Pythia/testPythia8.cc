/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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

#include <corsika/utl/CorsikaFenv.h>
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

    random::RNGManager::GetInstance().RegisterRandomStream("pythia");

    process::pythia::Decay model;
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

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}

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
    feenableexcept(FE_INVALID);
    setup::Stack stack;
    const HEPEnergyType E0 = 10_GeV;
    HEPMomentumType P0 =
        sqrt(E0 * E0 - particles::PiPlus::GetMass() * particles::PiPlus::GetMass());
    auto plab = corsika::stack::MomentumVector(cs, {0_GeV, 0_GeV, P0});
    geometry::Point pos(cs, 0_m, 0_m, 0_m);
    auto particle = stack.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::PiPlus, E0, plab, pos, 0_ns});

    random::RNGManager::GetInstance().RegisterRandomStream("pythia");

    corsika::stack::SecondaryView view(particle);
    auto projectile = view.GetProjectile();

    process::pythia::Decay model;

    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
    model.DoDecay(projectile);
    CHECK(stack.GetSize() == 3);
    auto const pSum = sumMomentum(view, cs);
    CHECK((pSum - plab).norm() / 1_GeV == Approx(0).margin(1e-4));
    CHECK((pSum.norm() - plab.norm()) / 1_GeV == Approx(0).margin(1e-4));
  }

  SECTION("pythia decay config") {
    process::pythia::Decay model({particles::Code::PiPlus, particles::Code::PiMinus});
    REQUIRE(model.IsDecayHandled(particles::Code::PiPlus));
    REQUIRE(model.IsDecayHandled(particles::Code::PiMinus));
    REQUIRE_FALSE(model.IsDecayHandled(particles::Code::KPlus));

    const std::vector<particles::Code> particleTestList = {
        particles::Code::PiPlus, particles::Code::PiMinus, particles::Code::KPlus,
        particles::Code::Lambda0Bar, particles::Code::D0Bar};

    // setup decays
    model.SetHandleDecay(particleTestList);
    for (auto& pCode : particleTestList) REQUIRE(model.IsDecayHandled(pCode));

    // individually
    model.SetHandleDecay(particles::Code::KMinus);

    // possible decays
    REQUIRE_FALSE(model.CanHandleDecay(particles::Code::Proton));
    REQUIRE_FALSE(model.CanHandleDecay(particles::Code::Electron));
    REQUIRE(model.CanHandleDecay(particles::Code::PiPlus));
    REQUIRE(model.CanHandleDecay(particles::Code::MuPlus));
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

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(projectile);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }
}
