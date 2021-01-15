/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/urqmd/UrQMD.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/utility/CorsikaFenv.hpp>

#include <SetupTestStack.hpp>
#include <SetupTestEnvironment.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <tuple>
#include <utility>

#include <catch2/catch.hpp>

/*
  NOTE, WARNING, ATTENTION

  The urqmd/Random.hpp implements the hook of urqmd to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below in multiple "tests" and
  link them togehter, it will fail.
 */
#include <corsika/modules/urqmd/Random.hpp>

using namespace corsika;
using namespace corsika::urqmd;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;
  for (auto const& p : view) { totalCharge += get_charge_number(p.getPID()); }
  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, CoordinateSystemPtr const& vCS) {
  MomentumVector sum{vCS, 0_eV, 0_eV, 0_eV};
  for (auto const& p : view) { sum += p.getMomentum(); }
  return sum;
}

TEST_CASE("UrQMD") {

  logging::set_level(logging::level::info);
  corsika_logger->set_pattern("[%n:%^%-8l%$] custom pattern: %v");

  SECTION("conversion") {
    CHECK_THROWS(corsika::urqmd::convertFromUrQMD(106, 0));
    CHECK(corsika::urqmd::convertFromUrQMD(101, 0) == Code::Pi0);
    CHECK(corsika::urqmd::convertToUrQMD(Code::PiPlus) ==
          std::make_pair<int, int>(101, 2));
  }

  feenableexcept(FE_INVALID);
  RNGManager::getInstance().registerRandomStream("urqmd");
  UrQMD urqmd;

  SECTION("interaction length") {
    auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Nitrogen);
    auto const& cs = *csPtr;
    { [[maybe_unused]] auto const& env_dummy = env; }

    Code validProjectileCodes[] = {Code::PiPlus,  Code::PiMinus, Code::Proton,
                                   Code::Neutron, Code::KPlus,   Code::KMinus,
                                   Code::K0,      Code::K0Bar,   Code::K0Long};

    for (auto code : validProjectileCodes) {
      auto [stack, view] = setup::testing::setup_stack(
          code, 0, 0, 100_GeV, (setup::Environment::BaseNodeType* const)nodePtr, cs);
      CHECK(stack->getEntries() == 1);
      CHECK(view->getEntries() == 0);

      // simple check whether the cross-section is non-vanishing
      CHECK(urqmd.getCrossSection(view->getProjectile(), Code::Proton) / 1_mb > 0);
      CHECK(urqmd.getCrossSection(view->getProjectile(), Code::Nitrogen) / 1_mb > 0);
      CHECK(urqmd.getCrossSection(view->getProjectile(), Code::Oxygen) / 1_mb > 0);
      CHECK(urqmd.getCrossSection(view->getProjectile(), Code::Argon) / 1_mb > 0);
    }
  }

  SECTION("nucleus projectile") {
    auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::Nucleus, A, Z, 40_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    CHECK(stackPtr->getEntries() == 1);
    CHECK(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();
    urqmd.doInteraction(*secViewPtr);

    CHECK(sumCharge(*secViewPtr) == Z + get_charge_number(Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.getCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).getNorm() / projectileMomentum.getNorm() ==
          Approx(0).margin(1e-2));
  }

  SECTION("\"special\" projectile") {
    auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::PiPlus, 0, 0, 40_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    CHECK(stackPtr->getEntries() == 1);
    CHECK(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    urqmd.doInteraction(*secViewPtr);

    CHECK(sumCharge(*secViewPtr) ==
          get_charge_number(Code::PiPlus) + get_charge_number(Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.getCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).getNorm() / projectileMomentum.getNorm() ==
          Approx(0).margin(1e-2));
  }

  SECTION("K0Long projectile") {
    auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
        Code::K0Long, 0, 0, 40_GeV, (setup::Environment::BaseNodeType* const)nodePtr,
        *csPtr);
    CHECK(stackPtr->getEntries() == 1);
    CHECK(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->getProjectile();
    auto const projectileMomentum = projectile.getMomentum();

    urqmd.doInteraction(*secViewPtr);

    CHECK(sumCharge(*secViewPtr) ==
          get_charge_number(Code::K0Long) + get_charge_number(Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.getCoordinateSystem());
    CHECK((secMomSum - projectileMomentum).getNorm() / projectileMomentum.getNorm() ==
          Approx(0).margin(1e-2));
  }
}
