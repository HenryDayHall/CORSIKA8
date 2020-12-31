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

#include <SetupTestEnvironment.hpp>
#include <SetupTestStack.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

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

    std::set<Code> const particleList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                         Code::KMinus, Code::K0Long,  Code::K0Short};
    RNGManager::getInstance().registerRandomStream("pythia");
    corsika::pythia8::Decay model(particleList);
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

template <typename TStackView>
auto sumMomentum(TStackView const& view, CoordinateSystemPtr const& vCS) {
  MomentumVector sum{vCS, 0_eV, 0_eV, 0_eV};
  for (auto const& p : view) { sum += p.getMomentum(); }
  return sum;
}

TEST_CASE("pythia process") {

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Proton);
  auto const& cs = *csPtr;
  [[maybe_unused]] auto const& env_dummy = env;
  [[maybe_unused]] auto const& node_dummy = nodePtr;

  SECTION("pythia decay") {
    HEPEnergyType const P0 = 10_GeV;
    //HEPMomentumType const E0 = sqrt(P0*P0 + PiPlus::mass*PiPlus::mass);

    // feenableexcept(FE_INVALID); \todo how does this work nowadays...???
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::PiPlus, 0, 0, P0, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    auto& stack = *stackPtr;
    auto& view = *secViewPtr;

    auto const& particle = stack.getNextParticle();
    auto const  plab = MomentumVector(cs, {P0, 0_GeV, 0_GeV});

    std::set<Code> const particleList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                         Code::KMinus, Code::K0Long,  Code::K0Short};

    RNGManager::getInstance().registerRandomStream("pythia");

    corsika::pythia8::Decay model(particleList);

    [[maybe_unused]] const TimeType time = model.getLifetime(particle);
    model.doDecay(*secViewPtr);
    CHECK(stack.getEntries() == 3);
    auto const pSum = sumMomentum(view, cs);
    CHECK((pSum - plab).getNorm() / 1_GeV == Approx(0).margin(1e-4));
    CHECK((pSum.getNorm() - plab.getNorm()) / 1_GeV == Approx(0).margin(1e-4));
  }

  SECTION("pythia decay config") {
    corsika::pythia8::Decay model({Code::PiPlus, Code::PiMinus});
    CHECK(model.isDecayHandled(Code::PiPlus));
    CHECK(model.isDecayHandled(Code::PiMinus));
    CHECK_FALSE(model.isDecayHandled(Code::KPlus));

    const std::vector<Code> particleTestList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                                Code::Lambda0Bar, Code::D0Bar};

    // setup decays
    model.setHandleDecay(particleTestList);
    for (auto& pCode : particleTestList) CHECK(model.isDecayHandled(pCode));

    // individually
    model.setHandleDecay(Code::KMinus);

    // possible decays
    CHECK_FALSE(model.canHandleDecay(Code::Proton));
    CHECK_FALSE(model.canHandleDecay(Code::Electron));
    CHECK(model.canHandleDecay(Code::PiPlus));
    CHECK(model.canHandleDecay(Code::MuPlus));
  }

  SECTION("pythia interaction") {

    // feenableexcept(FE_INVALID); \todo how does this work nowadays
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::PiPlus, 0, 0, 100_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    auto& view = *secViewPtr;
    auto particle = stackPtr->first();

    corsika::pythia8::Interaction model;
    model.doInteraction(view);
    [[maybe_unused]] const GrammageType length = model.getInteractionLength(particle);
    CHECK(length / 1_kg * square(1_m) == Approx(82.2524));
  }
}
