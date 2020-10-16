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
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/UniformMagneticField.h>

using namespace corsika;
using namespace corsika::units::si;

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}

TEST_CASE("pythia process") {

  auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Proton);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  SECTION("pythia decay") {
    feenableexcept(FE_INVALID);
    const HEPEnergyType P0 = 10_GeV;
    auto [stackPtr, secViewPtr] =
        setup::testing::setupStack(particles::Code::PiPlus, 0, 0, P0, nodePtr, *csPtr);
    const auto plab = corsika::stack::MomentumVector(
        cs, {P0, 0_eV, 0_eV}); // this is secret knowledge about setupStack
    auto& stack = *stackPtr;
    auto& view = *secViewPtr;
    auto particle = stackPtr->first();

    random::RNGManager::GetInstance().RegisterRandomStream("pythia");

    process::pythia::Decay model;

    [[maybe_unused]] const TimeType time = model.GetLifetime(particle);
    model.DoDecay(view);
    CHECK(stack.getEntries() == 3);
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

    feenableexcept(FE_INVALID);
    auto [stackPtr, secViewPtr] = setup::testing::setupStack(particles::Code::PiPlus, 0,
                                                             0, 100_GeV, nodePtr, *csPtr);
    auto& view = *secViewPtr;
    auto particle = stackPtr->first();

    process::pythia::Interaction model;

    [[maybe_unused]] const process::EProcessReturn ret = model.DoInteraction(view);
    [[maybe_unused]] const GrammageType length = model.GetInteractionLength(particle);
  }
}
