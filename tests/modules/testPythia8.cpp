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

using DummyEnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
using DummyEnvironment = Environment<DummyEnvironmentInterface>;

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
    RNGManager<>::getInstance().registerRandomStream("pythia");
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
  MomentumVector sum{vCS};
  for (auto const& p : view) { sum += p.getMomentum(); }
  return sum;
}

TEST_CASE("Pythia8Interface", "modules") {

  logging::set_level(logging::level::info);

  auto const rootCS = get_root_CoordinateSystem();
  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Proton);
  auto const& cs = *csPtr;
  {
    [[maybe_unused]] auto const& env_dummy = env;
    [[maybe_unused]] auto const& node_dummy = nodePtr;
  }

  SECTION("pythia decay") {
    HEPEnergyType const P0 = 10_GeV;
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::PiPlus, P0, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
    auto& stack = *stackPtr;
    auto& view = *secViewPtr;

    auto const& particle = stack.getNextParticle();
    auto const plab = MomentumVector(cs, {P0, 0_GeV, 0_GeV});

    std::set<Code> const particleList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                         Code::KMinus, Code::K0Long,  Code::K0Short};

    RNGManager<>::getInstance().registerRandomStream("pythia");

    corsika::pythia8::Decay decay(particleList);

    CORSIKA_LOG_INFO("stack: {} {}", stack.asString(), particle.asString());
    [[maybe_unused]] const TimeType time = decay.getLifetime(particle);
    double const gamma = particle.getEnergy() / get_mass(Code::PiPlus);
    REQUIRE(time == get_lifetime(Code::PiPlus) * gamma);
    decay.doDecay(*secViewPtr);
    CORSIKA_LOG_INFO("piplus->{}", stack.asString());
    REQUIRE(stack.getEntries() == 3); // piplus, muplu, numu
    auto const pSum = sumMomentum(view, cs);
    REQUIRE((pSum - plab).getNorm() / 1_GeV == Approx(0).margin(1e-4));
    REQUIRE((pSum.getNorm() - plab.getNorm()) / 1_GeV == Approx(0).margin(1e-4));
  }

  SECTION("pythia decay config") {
    corsika::pythia8::Decay decay({Code::PiPlus, Code::PiMinus});
    REQUIRE(decay.isDecayHandled(Code::PiPlus));
    REQUIRE(decay.isDecayHandled(Code::PiMinus));
    REQUIRE_FALSE(decay.isDecayHandled(Code::KPlus));

    const std::vector<Code> particleTestList = {Code::PiPlus, Code::PiMinus, Code::KPlus,
                                                Code::Lambda0Bar, Code::D0Bar};

    // setup decays
    decay.setHandleDecay(particleTestList);
    for (auto& pCode : particleTestList) REQUIRE(decay.isDecayHandled(pCode));

    // individually
    decay.setHandleDecay(Code::KMinus);

    // impossible
    REQUIRE_THROWS(decay.setHandleDecay(Code::Photon));

    REQUIRE(decay.isDecayHandled(Code::PiPlus));
    REQUIRE_FALSE(decay.isDecayHandled(Code::Photon));

    // possible decays
    REQUIRE_FALSE(decay.canHandleDecay(Code::Photon));
    REQUIRE_FALSE(decay.canHandleDecay(Code::Proton));
    REQUIRE_FALSE(decay.canHandleDecay(Code::Electron));
    REQUIRE(decay.canHandleDecay(Code::PiPlus));
    REQUIRE(decay.canHandleDecay(Code::MuPlus));
  }

  SECTION("pythia interaction") {

    // this will be a p-p collision at sqrts=3.5TeV -> no problem for pythia
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Proton, 7_TeV, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
    auto& view = *secViewPtr;

    corsika::pythia8::Interaction collision;

    REQUIRE(collision.canInteract(Code::Proton));
    REQUIRE(collision.canInteract(Code::AntiProton));
    REQUIRE(collision.canInteract(Code::Neutron));
    REQUIRE(collision.canInteract(Code::AntiNeutron));
    REQUIRE(collision.canInteract(Code::PiMinus));
    REQUIRE(collision.canInteract(Code::PiPlus));
    REQUIRE_FALSE(collision.canInteract(Code::Electron));

    // pi+p
    REQUIRE(collision.getCrossSection(
                Code::PiPlus, Code::Proton,
                {sqrt(static_pow<2>(PiPlus::mass) + static_pow<2>(100_GeV)),
                 {rootCS, {0_eV, 0_eV, 100_GeV}}},
                {Proton::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) > 0_mb);

    // pi+H
    REQUIRE(collision.getCrossSection(
                Code::PiPlus, Code::Hydrogen,
                {sqrt(static_pow<2>(PiPlus::mass) + static_pow<2>(100_GeV)),
                 {rootCS, {0_eV, 0_eV, 100_GeV}}},
                {Hydrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) > 0_mb);

    // K+N
    REQUIRE(collision.getCrossSection(
                Code::KPlus, Code::Nitrogen,
                {sqrt(static_pow<2>(KPlus::mass) + static_pow<2>(100_GeV)),
                 {rootCS, {0_eV, 0_eV, 100_GeV}}},
                {Nitrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) > 0_mb);

    collision.doInteraction(view, Code::Proton, Code::Nitrogen,
                            {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
                             {rootCS, {0_eV, 0_eV, 100_GeV}}},
                            {Nitrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}});
    REQUIRE(view.getSize() >= 2);
  }

  SECTION("pythia too low energy") {

    // this is a projectile neutron with very little energy
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Neutron, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
    auto& view = *secViewPtr;

    corsika::pythia8::Interaction collision;

    REQUIRE_THROWS(collision.doInteraction(
        view, Code::Neutron, Code::Hydrogen,
        {sqrt(static_pow<2>(Neutron::mass) + static_pow<2>(1_MeV)),
         {rootCS, {0_eV, 0_eV, 1_MeV}}},
        {Hydrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}));
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
        Code::Proton, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr_Fe,
        *csPtr_Fe);
    auto& view = *secViewPtr;
    { [[maybe_unused]] auto const& dummy_StackPtr = stackPtr; }

    corsika::pythia8::Interaction collision;

    REQUIRE(collision.getCrossSectionInelEla(
                Code::Proton, Code::Iron,
                {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
                 {rootCS, {0_eV, 0_eV, 100_GeV}}},
                {Iron::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) == std::tuple{0_mb, 0_mb});

    REQUIRE(collision.getCrossSection(
                Code::Proton, Code::Iron,
                {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
                 {rootCS, {0_eV, 0_eV, 100_GeV}}},
                {Iron::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) == 0_mb);

    REQUIRE_THROWS(collision.doInteraction(
        view, Code::Proton, Code::Iron,
        {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
         {rootCS, {0_eV, 0_eV, 100_GeV}}},
        {Iron::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}));
  }

  SECTION("pythia wrong projectile") {

    // resonable projectile, but tool low energy
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Iron, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
    { [[maybe_unused]] auto const& dummy_StackPtr = stackPtr; }

    corsika::pythia8::Interaction collision;
    REQUIRE(collision.getCrossSectionInelEla(
              Code::Helium, Code::Nitrogen,
              {sqrt(static_pow<2>(Helium::mass) + static_pow<2>(100_GeV)),
               {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Nitrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) == std::tuple{0_mb, 0_mb});

    REQUIRE_THROWS(
        collision.doInteraction(*secViewPtr, Code::Helium, Code::Nitrogen,
                                {sqrt(static_pow<2>(Helium::mass) + static_pow<2>(100_GeV)),
                                 {rootCS, {0_eV, 0_eV, 100_GeV}}},
                                {Nitrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}));
  }
}
