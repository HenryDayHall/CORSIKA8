/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/stack/CombinedStack.h>
#include <corsika/stack/dummy/DummyStack.h>
#include <corsika/stack/node/GeometryNodeStackExtension.h>

using namespace corsika;
using namespace corsika::stack;

#include <catch2/catch.hpp>

#include <iostream>
using namespace std;

// this is our dummy environment, it only knows its trivial BaseNodeType
class DummyEnv {
public:
  typedef int BaseNodeType;
};

// the GeometryNode stack needs to know the type of geometry-nodes from the DummyEnv:
template <typename TStackIter>
using DummyGeometryDataInterface =
    typename corsika::stack::node::MakeGeometryDataInterface<TStackIter, DummyEnv>::type;

// combine dummy stack with geometry information for tracking
template <typename TStackIter>
using StackWithGeometryInterface =
    corsika::stack::CombinedParticleInterface<dummy::DummyStack::PIType,
                                              DummyGeometryDataInterface, TStackIter>;

using TestStack =
    corsika::stack::CombinedStack<typename stack::dummy::DummyStack::StackImpl,
                                  stack::node::GeometryData<DummyEnv>,
                                  StackWithGeometryInterface>;

TEST_CASE("GeometryNodeStackExtension", "[stack]") {

  dummy::NoData noData;

  SECTION("write node") {

    const int data = 5;

    TestStack s;
    s.AddParticle(std::tuple<dummy::NoData>{noData}, std::tuple<const int*>{&data});

    CHECK(s.GetSize() == 1);
  }

  SECTION("write/read node") {
    const int data = 15;

    TestStack s;
    auto p = s.AddParticle(std::tuple<dummy::NoData>{noData});
    p.SetNode(&data);
    CHECK(s.GetSize() == 1);

    const auto pout = s.GetNextParticle();
    CHECK(*(pout.GetNode()) == 15);
  }

  SECTION("stack fill and cleanup") {

    const int data = 16;

    TestStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      auto p = s.AddParticle(std::tuple<dummy::NoData>{noData});
      p.SetNode(&data);
    }

    CHECK(s.GetSize() == 99);
    double v = 0;
    for (int i = 0; i < 99; ++i) {
      auto p = s.GetNextParticle();
      v += *(p.GetNode());
      p.Delete();
    }
    CHECK(v == 99 * data);
    CHECK(s.GetSize() == 0);
  }
}
