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

TEST_CASE("Pythia8", "modules") {

  logging::set_level(logging::level::info);

  SECTION("linking pythia") {
    using namespace Pythia8;

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
      CORSIKA_LOG_CRITICAL("decay failed!");
    else
      CORSIKA_LOG_DEBUG("particles after decay: {}", event.size());
    event.list();

    // loop over final state
    for (int i = 0; i < pythia.event.size(); ++i)
      if (pythia.event[i].isFinal()) {
        CORSIKA_LOG_DEBUG("particle: id= {}", pythia.event[i].id());
      }
  }

  SECTION("pythia interface") {

    std::set<Code> const particleList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                         Code::KMinus, Code::K0Long,  Code::K0Short};
    RNGManager::getInstance().registerRandomStream("pythia");
    corsika::pythia8::Decay decay(particleList);
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

TEST_CASE("Pythia8Interface", "modules") {

  logging::set_level(logging::level::info);
  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Proton);
  auto const& cs = *csPtr;
  {
    [[maybe_unused]] auto const& env_dummy = env;
    [[maybe_unused]] auto const& node_dummy = nodePtr;
  }

  SECTION("pythia decay") {
    HEPEnergyType const P0 = 10_GeV;
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::PiPlus, 0, 0, P0, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    auto& stack = *stackPtr;
    auto& view = *secViewPtr;

    auto const& particle = stack.getNextParticle();
    auto const plab = MomentumVector(cs, {P0, 0_GeV, 0_GeV});

    std::set<Code> const particleList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                         Code::KMinus, Code::K0Long,  Code::K0Short};

    RNGManager::getInstance().registerRandomStream("pythia");

    corsika::pythia8::Decay decay(particleList);

    [[maybe_unused]] const TimeType time = decay.getLifetime(particle);
    double const gamma = particle.getEnergy() / get_mass(Code::PiPlus);
    CHECK(time == get_lifetime(Code::PiPlus) * gamma);
    decay.doDecay(*secViewPtr);
    CHECK(stack.getEntries() == 3); // piplus, muplu, numu
    auto const pSum = sumMomentum(view, cs);
    CHECK((pSum - plab).getNorm() / 1_GeV == Approx(0).margin(1e-4));
    CHECK((pSum.getNorm() - plab.getNorm()) / 1_GeV == Approx(0).margin(1e-4));
  }

  SECTION("pythia decay config") {
    corsika::pythia8::Decay decay({Code::PiPlus, Code::PiMinus});
    CHECK(decay.isDecayHandled(Code::PiPlus));
    CHECK(decay.isDecayHandled(Code::PiMinus));
    CHECK_FALSE(decay.isDecayHandled(Code::KPlus));

    const std::vector<Code> particleTestList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                                Code::Lambda0Bar, Code::D0Bar};

    // setup decays
    decay.setHandleDecay(particleTestList);
    for (auto& pCode : particleTestList) CHECK(decay.isDecayHandled(pCode));

    // individually
    decay.setHandleDecay(Code::KMinus);

    // impossible
    CHECK_THROWS(decay.setHandleDecay(Code::Photon));

    CHECK(decay.isDecayHandled(Code::PiPlus));
    CHECK_FALSE(decay.isDecayHandled(Code::Photon));

    // possible decays
    CHECK_FALSE(decay.canHandleDecay(Code::Photon));
    CHECK_FALSE(decay.canHandleDecay(Code::Proton));
    CHECK_FALSE(decay.canHandleDecay(Code::Electron));
    CHECK(decay.canHandleDecay(Code::PiPlus));
    CHECK(decay.canHandleDecay(Code::MuPlus));
  }

  SECTION("pythia interaction") {

    // this will be a p-p collision at sqrts=3.5TeV -> no problem for pythia
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Proton, 0, 0, 7_TeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    auto& view = *secViewPtr;
    auto particle = stackPtr->first();

    corsika::pythia8::Interaction collision;

    CHECK(collision.canInteract(Code::Proton));
    CHECK(collision.canInteract(Code::AntiProton));
    CHECK(collision.canInteract(Code::Neutron));
    CHECK(collision.canInteract(Code::AntiNeutron));
    CHECK(collision.canInteract(Code::PiMinus));
    CHECK(collision.canInteract(Code::PiPlus));
    CHECK_FALSE(collision.canInteract(Code::Electron));

    // nuclei not supported
    CHECK_THROWS(collision.getCrossSection(Code::Proton, Code::Helium, 1_TeV));
    std::tuple<CrossSectionType, CrossSectionType> xs_test =
        collision.getCrossSection(Code::Iron, Code::Hydrogen, 1_GeV);
    CHECK(std::get<0>(xs_test) == std::numeric_limits<double>::infinity() * 1_mb);
    CHECK(std::get<1>(xs_test) == std::numeric_limits<double>::infinity() * 1_mb);

    collision.getInteractionLength(particle);

    collision.doInteraction(view);
    [[maybe_unused]] const GrammageType length = collision.getInteractionLength(particle);
    CHECK(length / 1_kg * square(1_m) == Approx(43.04).margin(5e-1));
    CHECK(view.getSize() == 38);
  }

  SECTION("pythia nucleus projectile") {

    // this is a projectile nucleus with very little energy
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Oxygen, 0, 0, 17_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    auto& view = *secViewPtr;
    auto particle = stackPtr->first();

    corsika::pythia8::Interaction collision;

    GrammageType lambda_test = collision.getInteractionLength(particle);
    CHECK(lambda_test == std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm));

    CHECK_THROWS(collision.doInteraction(view));
  }

  SECTION("pythia too low energy") {

    // this is a projectile neutron with very little energy
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Neutron, 0, 0, 1_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    auto& view = *secViewPtr;
    auto particle = stackPtr->first();

    corsika::pythia8::Interaction collision;

    GrammageType lambda_test = collision.getInteractionLength(particle);
    CHECK(lambda_test == std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm));

    CHECK_THROWS(collision.doInteraction(view));
  }

  SECTION("pythia wrong target") {

    // incompatible target
    auto [env_Fe, csPtr_Fe, nodePtr_Fe] = setup::testing::setup_environment(Code::Iron);
    {
      [[maybe_unused]] auto const& cs_Fe = *csPtr_Fe;
      [[maybe_unused]] auto const& env_dummy_Fe = env_Fe;
      [[maybe_unused]] auto const& node_dummy_Fe = nodePtr_Fe;
    }
    
    // resonable projectile, but tool low energy
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Proton, 0, 0, 1_GeV, (setup::Environment::BaseNodeType* const)nodePtr_Fe,
        *csPtr_Fe);
    auto& view = *secViewPtr;
    {
      [[maybe_unused]] auto const& dummy_StackPtr = stackPtr;
    }
    

    corsika::pythia8::Interaction collision;

    CHECK_THROWS(collision.doInteraction(view));
  }
}
