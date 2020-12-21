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

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/HomogeneousMedium.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <tuple>
#include <utility>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::urqmd;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;

  for (auto const& p : view) { totalCharge += corsika::charge_number(p.GetPID()); }

  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, corsika::CoordinateSystem const& vCS) {
  corsika::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}

TEST_CASE("UrQMD") {
  SECTION("conversion") {
    REQUIRE_THROWS(corsika::urqmd::ConvertFromUrQMD(106, 0));
    REQUIRE(corsika::urqmd::ConvertFromUrQMD(101, 0) == corsika::Code::Pi0);
    REQUIRE(corsika::urqmd::ConvertToUrQMD(corsika::Code::PiPlus) ==
            std::make_pair<int, int>(101, 2));
  }

  feenableexcept(FE_INVALID);
  corsika::RNGManager::getInstance().registerRandomStream("urqmd");
  UrQMD urqmd;

  SECTION("interaction length") {
    auto [env, csPtr, nodePtr] =
        setup::testing::setupEnvironment(particles::Code::Nitrogen);
    auto const& cs = *csPtr;
    { [[maybe_unused]] auto const& env_dummy = env; }

    corsika::Code validProjectileCodes[] = {
        corsika::Code::PiPlus,  corsika::Code::PiMinus, corsika::Code::Proton,
        corsika::Code::Neutron, corsika::Code::KPlus,   corsika::Code::KMinus,
        corsika::Code::K0,      corsika::Code::K0Bar,   corsika::Code::K0Long};

    for (auto code : validProjectileCodes) {
      auto [stack, view] = setup::testing::setupStack(code, 0, 0, 100_GeV, nodePtr, cs);
      REQUIRE(stack->getEntries() == 1);
      REQUIRE(view->getEntries() == 0);

      // simple check whether the cross-section is non-vanishing
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), corsika::Code::Proton) / 1_mb >
              0);
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), corsika::Code::Nitrogen) /
                  1_mb >
              0);
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), corsika::Code::Oxygen) / 1_mb >
              0);
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), corsika::Code::Argon) / 1_mb >
              0);
    }
  }

  SECTION("nucleus projectile") {
    auto [env, csPtr, nodePtr] =
        setup::testing::setupEnvironment(particles::Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setup::testing::setupStack(particles::Code::Nucleus, A,
                                                             Z, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();
    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) == Z + corsika::charge_number(corsika::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("\"special\" projectile") {
    auto [env, csPtr, nodePtr] =
        setup::testing::setupEnvironment(particles::Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] = setup::testing::setupStack(particles::Code::PiPlus, 0,
                                                             0, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) == corsika::charge_number(corsika::Code::PiPlus) +
                                          corsika::charge_number(corsika::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("K0Long projectile") {
    auto [env, csPtr, nodePtr] =
        setup::testing::setupEnvironment(particles::Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] = setup::testing::setupStack(particles::Code::K0Long, 0,
                                                             0, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) == corsika::charge_number(corsika::Code::K0Long) +
                                          corsika::charge_number(corsika::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }
}
