/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/process/InteractionCounter.hpp>
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

using namespace corsika;

const std::string refDataDir = std::string(REFDATADIR); // from cmake

struct DummyProcess {
  template <typename TParticle>
  GrammageType getInteractionLength([[maybe_unused]] TParticle const& particle) {
    return 100_g / 1_cm / 1_cm;
  }

  template <typename TParticle>
  void doInteraction([[maybe_unused]] TParticle& projectile) {}
};

TEST_CASE("InteractionCounter", "[process]") {

  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");
  logging::set_level(logging::level::info);

  DummyProcess d;
  InteractionCounter countedProcess(d);

  SECTION("getInteractionLength") {
    CHECK(countedProcess.getInteractionLength(nullptr) == 100_g / 1_cm / 1_cm);
  }

  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  [[maybe_unused]] auto& env_dummy = env;

  SECTION("DoInteraction nucleus") {
    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Nucleus, A, Z, 105_TeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    CHECK(stackPtr->getEntries() == 1);
    CHECK(secViewPtr->getEntries() == 0);

    countedProcess.doInteraction(*secViewPtr);

    auto const& h = countedProcess.getHistogram().labHist();
    CHECK(h.at(h.axis(0).index(1'000'070'140), h.axis(1).index(1.05e14)) == 1);
    CHECK(std::accumulate(h.cbegin(), h.cend(), 0) == 1);

    auto const& h2 = countedProcess.getHistogram().CMSHist();
    CHECK(h2.at(h2.axis(0).index(1'000'070'140), h2.axis(1).index(1.6e12)) == 1);
    CHECK(std::accumulate(h2.cbegin(), h2.cend(), 0) == 1);

    countedProcess.getHistogram().saveLab("testInteractionCounter_file1.npz",
                                          SaveMode::overwrite);
    countedProcess.getHistogram().saveCMS("testInteractionCounter_file2.npz",
                                          SaveMode::overwrite);
  }

  SECTION("DoInteraction Lambda") {
    auto constexpr code = Code::Lambda0;
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        code, 0, 0, 105_TeV, (setup::Environment::BaseNodeType* const)nodePtr, *csPtr);
    CHECK(stackPtr->getEntries() == 1);
    CHECK(secViewPtr->getEntries() == 0);

    countedProcess.doInteraction(*secViewPtr);

    auto const& h = countedProcess.getHistogram().labHist();
    CHECK(h.at(h.axis(0).index(3122), h.axis(1).index(1.05e14)) == 1);
    CHECK(std::accumulate(h.cbegin(), h.cend(), 0) == 1);

    auto const& h2 = countedProcess.getHistogram().CMSHist();
    CHECK(h2.at(h2.axis(0).index(3122), h2.axis(1).index(1.6e12)) == 1);
    CHECK(std::accumulate(h2.cbegin(), h2.cend(), 0) == 1);
  }
}

#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>

TEST_CASE("InteractionCounterOutput", "[output validation]") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

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
