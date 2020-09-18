/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/history/HistoryStackExtension.h>
#include <corsika/history/Event.hpp>
#include <corsika/history/HSecondaryView.hpp>

#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/dummy/DummyStack.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>

#include <catch2/catch.hpp>

#include <iostream>

using namespace corsika;
using namespace corsika::stack;
using namespace corsika::geometry;
using namespace corsika::units::si;

/**
   Need to replicate setup::SetupStack in a maximally simplified
   way, but with real particle data
 */

// the GeometryNode stack needs to know the type of geometry-nodes from the DummyEnv:
template <typename TStackIter>
using HistoryDataInterface =
    typename history::MakeHistoryDataInterface<TStackIter, history::Event>::type;

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithHistoryInterface = corsika::stack::CombinedParticleInterface<
    stack::nuclear_extension::ParticleDataStack::PIType, HistoryDataInterface,
    TStackIter>;

using TestStack = corsika::stack::CombinedStack<
    typename stack::nuclear_extension::ParticleDataStack::StackImpl,
    history::HistoryData<history::Event>, StackWithHistoryInterface>;

/*
    See Issue 161

    unfortunately clang does not support this in the same way (yet) as
    gcc, so we have to distinguish here. If clang cataches up, we
    could remove the clang branch here and also in
    corsika::Cascade. The gcc code is much more generic and
    universal. If we could do the gcc version, we won't had to define
    StackView globally, we could do it with MakeView whereever it is
    actually needed. Keep an eye on this!
  */
#if defined(__clang__)
using TestStackView = corsika::stack::SecondaryView<typename TestStack::StackImpl,
                                                    StackWithHistoryInterface>;
#elif defined(__GNUC__) || defined(__GNUG__)
using TestStackView = corsika::stack::MakeView<TestStack>::type;
#endif

TEST_CASE("HistoryStackExtension", "[stack]") {

  geometry::CoordinateSystem& dummyCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  TestStack s;

  SECTION("add lone particle") {

    auto p = s.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    CHECK(s.GetSize() == 1);
    corsika::history::EvtPtr evt = p.GetEvent();
    CHECK(evt == nullptr);
  }

  SECTION("generate event") {
    auto p = s.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    history::HSecondaryView<TestStackView> hview(p);

    hview.AddSecondary(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    // ... continue actual real test here an below ...
  }

  // REQUIRE(pout.GetPID() == particles::Code::Electron);
}
