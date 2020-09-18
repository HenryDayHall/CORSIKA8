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

#include <catch2/catch.hpp>

#include <iostream>

using namespace corsika;
using namespace corsika::stack;

// this is our dummy environment, it only knows its trivial BaseNodeType
class DummyEvent {
private:
  size_t parent_;
  std::vector<int> secondaries_;

public:
  DummyEvent() {}
  DummyEvent(const size_t parent) { parent_ = parent; }

  size_t getParentIndex() { return parent_; }
  void addSecondary(const int particle) { secondaries_.push_back(particle); }
  int multiplicity() const { return secondaries_.size(); }
};

// the GeometryNode stack needs to know the type of geometry-nodes from the DummyEnv:
template <typename TStackIter>
using DummyHistoryDataInterface =
    typename history::MakeHistoryDataInterface<TStackIter, DummyEvent>::type;

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithHistoryInterface =
    corsika::stack::CombinedParticleInterface<dummy::DummyStack::PIType,
                                              DummyHistoryDataInterface, TStackIter>;

using TestStack =
    corsika::stack::CombinedStack<typename stack::dummy::DummyStack::StackImpl,
                                  history::HistoryData<DummyEvent>,
                                  StackWithHistoryInterface>;

using EvtPtr = std::shared_ptr<DummyEvent>;

TEST_CASE("HistoryStackExtension", "[stack]") {

  const dummy::NoData noData;
  TestStack s;

  auto p = s.AddParticle(std::tuple<dummy::NoData>{noData});

  SECTION("add lone particle") {
    CHECK(s.GetSize() == 1);

    EvtPtr evt = p.GetEvent();
    CHECK(evt == nullptr);
  }
}
