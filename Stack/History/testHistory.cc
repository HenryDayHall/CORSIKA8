/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/history/HistoryStackExtension.h>
#include <corsika/stack/dummy/DummyStack.h>
#include <corsika/stack/CombinedStack.h>

using namespace corsika;
using namespace corsika::stack;

#include <catch2/catch.hpp>

#include <iostream>
using namespace std;

// this is our dummy environment, it only knows its trivial BaseNodeType
class DummyEvent {
public:
  DummyEvent() {}
  DummyEvent(const std::shared_ptr<DummyEvent>& parent) {
    parent_ = parent;
    //parent.addSecondary();
  }

  std::shared_ptr<DummyEvent> getParent() { return parent_; }
  void addSecondary(const std::shared_ptr<DummyEvent>& particle) { secondaries_.push_back(particle); }

  int multiplicity() const { return secondaries_.size(); }
  
private:
  std::shared_ptr<DummyEvent> parent_;
  std::vector<std::shared_ptr<DummyEvent>> secondaries_;
};

// the GeometryNode stack needs to know the type of geometry-nodes from the DummyEnv:
template <typename TStackIter>
using DummyHistoryDataInterface = typename history::MakeHistoryDataInterface<TStackIter, DummyEvent>::type;


// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithHistoryInterface = corsika::stack::CombinedParticleInterface<
  dummy::DummyStack::PIType,
  DummyHistoryDataInterface, TStackIter>;

using TestStack = corsika::stack::CombinedStack<
  typename stack::dummy::DummyStack::StackImpl,
  history::HistoryData<DummyEvent>,
  StackWithHistoryInterface>;


TEST_CASE("HistoryStackExtension", "[stack]") {

  const dummy::NoData noData;
  
  SECTION("write event") {

    TestStack s;
    s.AddParticle(std::tuple<dummy::NoData>{noData});
    REQUIRE(s.GetSize() == 1);
  }

  //REQUIRE(pout.GetPID() == particles::Code::Electron);
}
