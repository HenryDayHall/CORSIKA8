/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/IntLengthModifyingProcess.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <SetupTestStack.hpp>
#include <SetupTestEnvironment.hpp>

#include <catch2/catch.hpp>

#include <numeric>
#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <cstdio>

using namespace corsika;

struct DummyProcess {
  template <typename TParticle>
  GrammageType getInteractionLength(TParticle const&) {
    return 100_g / 1_cm / 1_cm;
  }

  template <typename TArgument>
  void doInteraction(TArgument& arg) {
    arg = 3;
  }
};

struct DummyParticle {
  HEPEnergyType getEnergy() const { return 47_TeV; }

  Code getPID() const { return Code::MuPlus; }
};

TEST_CASE("IntLengthModifyingProcess", "[process]") {
  DummyProcess u;

  auto const modifier = [](GrammageType orig, Code, HEPEnergyType) -> GrammageType {
    return orig * 2;
  };

  IntLengthModifyingProcess mod{u, modifier};

  SECTION("getInteractionLength") {
    DummyParticle const p;
    REQUIRE(mod.getInteractionLength(p) == 200_g / 1_cm / 1_cm);
  }

  SECTION("doInteraction") {
    int k = 0;
    mod.doInteraction(k);
    REQUIRE(k == 3);
  }
}
