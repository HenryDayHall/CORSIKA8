/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/random/RNGManager.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/utl/CorsikaFenv.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <tuple>
#include <utility>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::UrQMD;
using namespace corsika::units::si;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;

  for (auto const& p : view) { totalCharge += particles::GetChargeNumber(p.GetPID()); }

  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}


TEST_CASE("UrQMD") {
  SECTION("conversion") {
    REQUIRE_THROWS(process::UrQMD::ConvertFromUrQMD(106, 0));
    REQUIRE(process::UrQMD::ConvertFromUrQMD(101, 0) == particles::Code::Pi0);
    REQUIRE(process::UrQMD::ConvertToUrQMD(particles::Code::PiPlus) ==
            std::make_pair<int, int>(101, 2));
  }

  feenableexcept(FE_INVALID);
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("urqmd");
  UrQMD urqmd;

  SECTION("interaction length") {
    auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Nitrogen);
    auto const& cs = *csPtr;
    [[maybe_unused]] auto const& env_dummy = env;
    [[maybe_unused]] auto const& node_dummy = nodePtr;

    particles::Code validProjectileCodes[] = {
        particles::Code::PiPlus,  particles::Code::PiMinus, particles::Code::Proton,
        particles::Code::Neutron, particles::Code::KPlus,   particles::Code::KMinus,
        particles::Code::K0,      particles::Code::K0Bar,   particles::Code::K0Long};

    for (auto code : validProjectileCodes) {
      auto [stack, view] = setup::testing::setupStack(code, 0,0, 100_GeV, nodePtr, cs);
      REQUIRE(stack->getEntries() == 1);
      REQUIRE(view->getEntries() == 0);

      // simple check whether the cross-section is non-vanishing
      // only nuclei with available tabluated data so far
      REQUIRE(urqmd.GetInteractionLength(stack->GetNextParticle()) > 1_g / square(1_cm));
    }
  }

  SECTION("nucleus projectile") {
    auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setup::testing::setupStack(particles::Code::Nucleus, A, Z, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();
    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) ==
            Z + particles::GetChargeNumber(particles::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("\"special\" projectile") {
    auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] =
      setup::testing::setupStack(particles::Code::PiPlus, 0,0, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) ==
            particles::GetChargeNumber(particles::Code::PiPlus) +
                particles::GetChargeNumber(particles::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("K0Long projectile") {
    auto [env, csPtr, nodePtr] = setup::testing::setupEnvironment(particles::Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] =
      setup::testing::setupStack(particles::Code::K0Long,0,0, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) ==
            particles::GetChargeNumber(particles::Code::K0Long) +
                particles::GetChargeNumber(particles::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }
}
