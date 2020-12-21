/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/stack/history/Event.hpp>
#include <corsika/stack/history/HistorySecondaryProducer.hpp>
#include <corsika/stack/history/HistoryStackExtension.hpp>

#include <corsika/framework/stack/CombinedStack.hpp>
#include <corsika/stack/DummyStack.hpp>
#include <corsika/stack/NuclearStackExtension.hpp>

#include <corsika/framework/logging/Logging.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

/**
   Need to replicate setup::SetupStack in a maximally simplified
   way, but with real particle data
 */

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithHistoryInterface = CombinedParticleInterface<
    nuclear_stack::ParticleDataStack::pi_type,
    history::HistoryEventDataInterface, TStackIter>;

using TestStack = CombinedStack<
    typename nuclear_stack::ParticleDataStack::stack_implementation_type,
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
using TheTestStackView = SecondaryView<typename TestStack::StackImpl,
                                                       StackWithHistoryInterface,
                                                       history::HistorySecondaryProducer>;
#elif defined(__GNUC__) || defined(__GNUG__)
using TheTestStackView =
    MakeView<TestStack, history::HistorySecondaryProducer>::type;
#endif

using TestStackView = TheTestStackView;

template <typename Event>
int count_generations(Event const* event) {
  int genCounter = 0;
  while (event) {
    event = event->parentEvent().get();
    genCounter++;
  }

  return genCounter;
}

TEST_CASE("HistoryStackExtensionView", "[stack]") {

  CoordinateSystemPtr const& dummyCS = get_root_CoordinateSystem();

  // in this test we only use one singel stack !
  TestStack stack;

  // add primary particle
  auto p0 = stack.addParticle(
      std::make_tuple(Code::Electron, 1.5_GeV,
                      MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                      Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

  CHECK(stack.getEntries() == 1);
  corsika::history::EventPtr evt = p0.getEvent();
  CHECK(evt == nullptr);
  CHECK(count_generations(evt.get()) == 0);

  SECTION("interface test, view") {

    // add secondaries, 1st generation
    TestStackView hview0(p0);

    auto const ev0 = p0.getEvent();
    CHECK(ev0 == nullptr);

    CORSIKA_LOG_DEBUG("loop VIEW");

    // add 5 secondaries
    for (int i = 0; i < 5; ++i) {
      auto sec = hview0.addSecondary(
          std::make_tuple(Code::Electron, 1.5_GeV,
                          MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

      CHECK(sec.getParentEventIndex() == i);
      CHECK(sec.getEvent() != nullptr);
      CHECK(sec.getEvent()->parentEvent() == nullptr);
      CHECK(count_generations(sec.getEvent().get()) == 1);
    }

    // read 1st genertion particle particle
    auto p1 = stack.getNextParticle();
    CHECK(count_generations(p1.getEvent().get()) == 1);

    TestStackView hview1(p1);

    auto const ev1 = p1.getEvent();

    // add second generation of secondaries
    // add 10 secondaries
    for (int i = 0; i < 10; ++i) {
      auto sec = hview1.addSecondary(
          std::make_tuple(Code::Electron, 1.5_GeV,
                          MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

      CHECK(sec.getParentEventIndex() == i);
      CHECK(sec.getEvent()->parentEvent() == ev1);
      CHECK(sec.getEvent()->parentEvent()->parentEvent() == ev0);

      CHECK(count_generations(sec.getEvent().get()) == 2);

      const auto org_projectile = stack.at(sec.getEvent()->projectileIndex());
      CHECK(org_projectile.getEvent() == sec.getEvent()->parentEvent());
    }

    // read 2nd genertion particle particle
    auto p2 = stack.getNextParticle();

    TestStackView hview2(p2);

    auto const ev2 = p2.getEvent();

    // add third generation of secondaries
    // add 15 secondaries
    for (int i = 0; i < 15; ++i) {
      CORSIKA_LOG_TRACE("loop, view: " + std::to_string(i));

      auto sec = hview2.addSecondary(
          std::make_tuple(Code::Electron, 1.5_GeV,
                          MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
      CORSIKA_LOG_TRACE("loop, ---- ");

      CHECK(sec.getParentEventIndex() == i);
      CHECK(sec.getEvent()->parentEvent() == ev2);
      CHECK(sec.getEvent()->parentEvent()->parentEvent() == ev1);
      CHECK(sec.getEvent()->parentEvent()->parentEvent()->parentEvent() == ev0);

      CHECK(count_generations(sec.getEvent().get()) == 3);
    }
  }

  SECTION("also test projectile access") {

    CORSIKA_LOG_TRACE("projectile test");

    // add secondaries, 1st generation
    TestStackView hview0(p0);
    auto proj0 = hview0.getProjectile();
    auto const ev0 = p0.getEvent();
    CHECK(ev0 == nullptr);

    CORSIKA_LOG_TRACE("loop");

    // add 5 secondaries
    for (int i = 0; i < 5; ++i) {
      CORSIKA_LOG_TRACE("loop " + std::to_string(i));
      auto sec = proj0.addSecondary(
          std::make_tuple(Code::Electron, 1.5_GeV,
                          MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

      CHECK(sec.getParentEventIndex() == i);
      CHECK(sec.getEvent() != nullptr);
      CHECK(sec.getEvent()->parentEvent() == nullptr);
    }
    CHECK(stack.getEntries() == 6);

    // read 1st genertion particle particle
    auto p1 = stack.getNextParticle();

    TestStackView hview1(p1);
    auto proj1 = hview1.getProjectile();
    auto const ev1 = p1.getEvent();

    // add second generation of secondaries
    // add 10 secondaries
    for (unsigned int i = 0; i < 10; ++i) {
      auto sec = proj1.addSecondary(
          std::make_tuple(Code::Electron, 1.5_GeV,
                          MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                          Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));

      CHECK(sec.getParentEventIndex() == int(i));
      CHECK(sec.getEvent()->parentEvent() == ev1);
      CHECK(sec.getEvent()->secondaries().size() == i + 1);
      CHECK(sec.getEvent()->parentEvent()->parentEvent() == ev0);

      const auto org_projectile = stack.at(sec.getEvent()->projectileIndex());
      CHECK(org_projectile.getEvent() == sec.getEvent()->parentEvent());
    }
    CHECK(stack.getEntries() == 16);
  }
}
