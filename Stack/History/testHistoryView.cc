/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/history/HistoryStackExtension.h>
#include <corsika/history/Event.hpp>
#include <corsika/history/HistorySecondaryView.hpp>

#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/dummy/DummyStack.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>

#include <catch2/catch.hpp>

#include <iostream>

using namespace corsika;
using namespace corsika::geometry;
using namespace corsika::units::si;

/**
   Need to replicate setup::SetupStack in a maximally simplified
   way, but with real particle data
 */

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithHistoryInterface = corsika::stack::CombinedParticleInterface<
    stack::nuclear_extension::ParticleDataStack::PIType,
    history::HistoryEventDataInterface, TStackIter>;

using TestStack = corsika::stack::CombinedStack<
    typename stack::nuclear_extension::ParticleDataStack::StackImpl,
    history::HistoryEventData, StackWithHistoryInterface>;

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

  // in this test we only use one singel stack !
  TestStack stack;

  // add primary particle
  auto p0 = stack.AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
          particles::Code::Electron, 1.5_GeV,
          corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

  CHECK(stack.getEntries() == 1);
  corsika::history::EventPtr evt = p0.GetEvent();
  CHECK(evt == nullptr);

  SECTION("interface test, view") {
  
    // add secondaries, 1st generation
    history::HistorySecondaryView<TestStackView> hview0(p0);

    auto const ev0 = p0.GetEvent();
    CHECK(ev0 == nullptr);
    
    // add 5 secondaries
    for (int i = 0; i < 5; ++i) {
      auto sec = hview0.AddSecondary(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

      CHECK(sec.GetParentEventIndex() == i);
      CHECK(sec.GetEvent() != nullptr);
      CHECK(sec.GetEvent()->parentEvent() == nullptr);
    }

    // read 1st genertion particle particle
    auto p1 = stack.GetNextParticle();

  history::HistorySecondaryView<TestStackView> hview1(p1);

  auto const ev1 = p1.GetEvent();

  // add second generation of secondaries
  // add 10 secondaries
  for (int i = 0; i < 10; ++i) {
    auto sec = hview1.AddSecondary(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    CHECK(sec.GetParentEventIndex() == i);
    CHECK(sec.GetEvent()->parentEvent() == ev1);
    CHECK(sec.GetEvent()->parentEvent()->parentEvent() == ev0);

    CHECK((stack.begin() + sec.GetEvent()->projectileIndex()).GetEvent() ==
          sec.GetEvent()->parentEvent());
  }

  // read 2nd genertion particle particle
  auto p2 = stack.GetNextParticle();

  history::HistorySecondaryView<TestStackView> hview2(p2);

  auto const ev2 = p2.GetEvent();

  // add third generation of secondaries
  // add 15 secondaries
  for (int i = 0; i < 15; ++i) {
    auto sec = hview2.AddSecondary(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});

    CHECK(sec.GetParentEventIndex() == i);
    CHECK(sec.GetEvent()->parentEvent() == ev2);
    CHECK(sec.GetEvent()->parentEvent()->parentEvent() == ev1);
    CHECK(sec.GetEvent()->parentEvent()->parentEvent()->parentEvent() == ev0);
  }

  /*
    Now, let's perform some history reading and checking based on p3

    p3 should have 20 secondaries, and a projectile (with 15
    secondaries), with another projectil (with 10 secondaries), with
    antother projectil (5 secondaries), with NO parent


  {
    auto test_ev3 = p3.GetEvent();
    auto test_sec3 = test_ev3->secondaries();
    CHECK(test_sec3.size() == 20);

    auto test_proj3 = test_ev3->projectile(s.begin());
    CHECK(test_proj3.GetEvent() == ev3);
    auto test_ev2 = test_ev3->parentEvent();
    auto test_sec2 = test_ev2->secondaries();
    CHECK(test_sec2.size() == 15);

    auto test_proj2 = test_ev2->projectile(s.begin());
    CHECK(test_proj2.GetEvent() == ev2);
    auto test_ev1 = test_ev2->parentEvent();
    auto test_sec1 = test_ev1->secondaries();
    CHECK(test_sec1.size() == 10);

    auto test_proj1 = test_ev1->projectile(s.begin());
    CHECK(test_proj1.GetEvent() == ev1);

    CHECK(test_proj1.GetEvent()->parentEvent() == ev0);
    CHECK(test_proj1.GetEvent()->parentEvent()->parentEvent() == nullptr);
  }
  */
  }

  SECTION("also test projectile access") {
    
    // add secondaries, 1st generation 
    history::HistorySecondaryView<TestStackView> hview0(p0);
    auto proj0 = hview0.GetProjectile();    
    auto const ev0 = p0.GetEvent();
    CHECK(ev0 == nullptr);
    
    // add 5 secondaries
    for (int i = 0; i < 5; ++i) {
      auto sec = proj0.AddSecondary(
				     std::tuple<particles::Code, units::si::HEPEnergyType,
				     corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
				       particles::Code::Electron, 1.5_GeV,
					 corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
					 Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
      
      CHECK(sec.GetParentEventIndex() == i);
      CHECK(sec.GetEvent() != nullptr);
      CHECK(sec.GetEvent()->parentEvent() == nullptr);
    }
    CHECK(stack.getEntries()==6);


    // read 1st genertion particle particle
    auto p1 = stack.GetNextParticle();

    history::HistorySecondaryView<TestStackView> hview1(p1);
    auto proj1 = hview1.GetProjectile();
    auto const ev1 = p1.GetEvent();

    // add second generation of secondaries
    // add 10 secondaries
    for (int i = 0; i < 10; ++i) {
      auto sec = proj1.AddSecondary(
				     std::tuple<particles::Code, units::si::HEPEnergyType,
				     corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
				       particles::Code::Electron, 1.5_GeV,
					 corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
					 Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
      
      CHECK(sec.GetParentEventIndex() == i);
      CHECK(sec.GetEvent()->parentEvent() == ev1);
      CHECK(sec.GetEvent()->parentEvent()->secondaries().size() == 10);
      CHECK(sec.GetEvent()->parentEvent()->parentEvent() == ev0);
      
      CHECK((stack.begin() + sec.GetEvent()->projectileIndex()).GetEvent() ==
	    sec.GetEvent()->parentEvent());
    }
    CHECK(stack.getEntries()==16);
    
  }
}
