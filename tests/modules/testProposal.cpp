/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#include <corsika/modules/PROPOSAL.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <SetupTestEnvironment.hpp>
#include <SetupTestStack.hpp>
#include <catch2/catch.hpp>
#include <tuple>

using namespace corsika;
using namespace corsika::proposal;

using DummyEnvironmentInterface = IMediumPropertyModel<IMagneticFieldModel<IMediumModel>>;
using DummyEnvironment = Environment<DummyEnvironmentInterface>;

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/media/UniformMagneticField.hpp>

TEST_CASE("ProposalInterface", "modules") {

  logging::set_level(logging::level::info);

  // the test environment
  auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
  auto const& cs = *csPtr;

  auto [stackPtr, viewPtr] = setup::testing::setup_stack(
      Code::Electron, 10_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr, cs);
  test::StackView& view = *viewPtr;

  RNGManager<>::getInstance().registerRandomStream("sibyll");
  RNGManager<>::getInstance().registerRandomStream("proposal");

  corsika::sibyll::Interaction hadModel;
  corsika::proposal::Interaction emModel(*env, hadModel);

  SECTION("InteractionInterface - cross section") {
    auto& stack = *stackPtr;
    auto particle = stack.first();
    FourMomentum P4(
        100_GeV,
        {cs, {sqrt(static_pow<2>(100_MeV) - static_pow<2>(Proton::mass)), 0_eV, 0_eV}});
    CHECK(emModel.getCrossSection(particle, Code::Proton, P4) == 0_mb);
  }

  SECTION("InteractionInterface - hadronic photon interaction") {
    auto& stack = *stackPtr;
    // auto particle = stack.first();
    FourMomentum P4(100_TeV, {cs, {100_TeV, 0_eV, 0_eV}});
    // finish successfully
    CHECK(emModel.doHadronicInteraction(view, cs, P4, Code::Oxygen) == ProcessReturn::Ok);
    CHECK(stack.getEntries() > 1);
  }
}