/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/interaction_counter/InteractionCounter.hpp>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupStack.h>

#include <catch2/catch.hpp>

#include <numeric>

using namespace corsika;
using namespace corsika::process::interaction_counter;
using namespace corsika::units;
using namespace corsika::units::si;

const std::string refDataDir = std::string(REFDATADIR);

struct DummyProcess {
  template <typename TParticle>
  GrammageType GetInteractionLength([[maybe_unused]] TParticle const& particle) {
    return 100_g / 1_cm / 1_cm;
  }

  template <typename TParticle>
  auto DoInteraction([[maybe_unused]] TParticle& projectile) {
    return nullptr;
  }
};

TEST_CASE("InteractionCounter", "[process]") {

  logging::SetLevel(logging::level::debug);

  DummyProcess d;
  InteractionCounter countedProcess(d);

  SECTION("GetInteractionLength") {
    REQUIRE(countedProcess.GetInteractionLength(nullptr) == 100_g / 1_cm / 1_cm);
  }

  auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
  [[maybe_unused]] auto& env_dummy = env;

  SECTION("DoInteraction nucleus") {
    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setup::testing::setupStack(particles::Code::Nucleus, A,
                                                             Z, 105_TeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    auto const ret = countedProcess.DoInteraction(*secViewPtr);
    REQUIRE(ret == nullptr);

    auto const& h = countedProcess.GetHistogram().labHist();
    REQUIRE(h.at(h.axis(0).index(1'000'070'140), h.axis(1).index(1.05e14)) == 1);
    REQUIRE(std::accumulate(h.cbegin(), h.cend(), 0) == 1);

    auto const& h2 = countedProcess.GetHistogram().CMSHist();
    REQUIRE(h2.at(h2.axis(0).index(1'000'070'140), h2.axis(1).index(1.6e12)) == 1);
    REQUIRE(std::accumulate(h2.cbegin(), h2.cend(), 0) == 1);

    countedProcess.GetHistogram().saveLab("testInteractionCounter_file1.npz",
                                          utl::SaveMode::overwrite);
    countedProcess.GetHistogram().saveCMS("testInteractionCounter_file2.npz",
                                          utl::SaveMode::overwrite);
  }

  SECTION("DoInteraction Lambda") {
    auto constexpr code = particles::Code::Lambda0;
    auto [stackPtr, secViewPtr] =
        setup::testing::setupStack(code, 0, 0, 105_TeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    auto const ret = countedProcess.DoInteraction(*secViewPtr);
    REQUIRE(ret == nullptr);

    auto const& h = countedProcess.GetHistogram().labHist();
    REQUIRE(h.at(h.axis(0).index(3122), h.axis(1).index(1.05e14)) == 1);
    REQUIRE(std::accumulate(h.cbegin(), h.cend(), 0) == 1);

    auto const& h2 = countedProcess.GetHistogram().CMSHist();
    REQUIRE(h2.at(h2.axis(0).index(3122), h2.axis(1).index(1.6e12)) == 1);
    REQUIRE(std::accumulate(h2.cbegin(), h2.cend(), 0) == 1);
  }
}

#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>

TEST_CASE("InteractionCounterOutput", "[output validation]") {

  auto file = GENERATE(as<std::string>{}, "testInteractionCounter_file1",
                       "testInteractionCounter_file2");

  SECTION(std::string("check saved data, ") + file + ".npz") {

    std::cout << file + ".npz vs " << refDataDir + "/" + file + "_REF.npz" << std::endl;

    // compare to binary reference data
    std::ifstream file1(file + ".npz");
    std::ifstream file1ref(refDataDir + "/" + file + "_REF.npz");

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin1ref(file1ref);

    std::istreambuf_iterator<char> end;

    while (begin1 != end && begin1ref != end) {
      CHECK(*begin1 == *begin1ref);
      ++begin1;
      ++begin1ref;
    }
    CHECK(begin1 == end);
    CHECK(begin1ref == end);
    file1.close();
    file1ref.close();
  }
}
