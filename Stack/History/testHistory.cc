/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/history/HistoryStackExtension.h>
#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/dummy/DummyStack.h>
#include <corsika/history/Event.hpp>
#include <corsika/history/HSecondaryView.hpp>

#include <catch2/catch.hpp>

#include <iostream>

using namespace corsika;
using namespace corsika::stack;

// the GeometryNode stack needs to know the type of geometry-nodes from the DummyEnv:
template <typename TStackIter>
using DummyHistoryDataInterface =
    typename history::MakeHistoryDataInterface<TStackIter, history::Event>::type;

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithHistoryInterface =
    corsika::stack::CombinedParticleInterface<dummy::DummyStack::PIType,
                                              DummyHistoryDataInterface, TStackIter>;

using TestStack =
    corsika::stack::CombinedStack<typename stack::dummy::DummyStack::StackImpl,
                                  history::HistoryData<history::Event>,
                                  StackWithHistoryInterface>;

using EvtPtr = std::shared_ptr<history::Event>;

TEST_CASE("HistoryStackExtension", "[stack]") {

  const dummy::NoData noData;
  TestStack s;

  auto p = s.AddParticle(std::tuple<dummy::NoData>{noData});

  SECTION("add lone particle") {
    CHECK(s.GetSize() == 1);

    EvtPtr evt = p.GetEvent();
    CHECK(evt == nullptr);
  }

  SECTION("write event") {
    history::HSecondaryView hview{p};

    hview.AddSecondary(std::tuple<dummy::NoData>{noData});
  }

  // REQUIRE(pout.GetPID() == particles::Code::Electron);
}
