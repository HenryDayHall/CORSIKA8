/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/framework/random/RNGManager.hpp>

#include <corsika/framework/geometry/Point.h>
#include <corsika/framework/geometry/RootCoordinateSystem.h>
#include <corsika/framework/geometry/Vector.h>

#include <corsika/units/PhysicalConstants.h>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/utl/CorsikaFenv.h>

#include <corsika/framework/coreParticleProperties.h>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <tuple>
#include <utility>

#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::UrQMD;
using namespace corsika::units::si;

template <typename TStackView>
auto sumCharge(TStackView const& view) {
  int totalCharge = 0;

  for (auto const& p : view) { totalCharge += GetChargeNumber(p.GetPID()); }

  return totalCharge;
}

template <typename TStackView>
auto sumMomentum(TStackView const& view, geometry::CoordinateSystem const& vCS) {
  geometry::Vector<hepenergy_d> sum{vCS, 0_eV, 0_eV, 0_eV};

  for (auto const& p : view) { sum += p.GetMomentum(); }

  return sum;
}

auto setupEnvironment(Code vTargetCode) {
  // setup environment, geometry
  auto env = std::make_unique<environment::Environment<environment::IMediumModel>>();
  auto& universe = *(env->GetUniverse());
  const geometry::CoordinateSystem& cs = env->GetCoordinateSystem();

  auto theMedium =
      environment::Environment<environment::IMediumModel>::CreateNode<geometry::Sphere>(
          geometry::Point{cs, 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(std::vector<Code>{vTargetCode},
                                      std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  return std::make_tuple(std::move(env), &cs, nodePtr);
}

template <typename TNodeType>
auto setupStack(int vA, int vZ, HEPEnergyType vMomentum, TNodeType* vNodePtr,
                geometry::CoordinateSystem const& cs) {
  auto stack = std::make_unique<setup::Stack>();
  auto constexpr mN = corsika::units::constants::nucleonMass;

  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  corsika::stack::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

  HEPEnergyType const E0 = sqrt(units::static_pow<2>(mN * vA) + pLab.squaredNorm());
  auto particle = stack->AddParticle(
      std::tuple<Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                 geometry::Point, units::si::TimeType, unsigned short, unsigned short>{
          Code::Nucleus, E0, pLab, origin, 0_ns, vA, vZ});

  particle.SetNode(vNodePtr);
  return std::make_tuple(
      std::move(stack), std::make_unique<decltype(setup::StackView{particle})>(particle));
}

template <typename TNodeType>
auto setupStack(Code vProjectileType, HEPEnergyType vMomentum, TNodeType* vNodePtr,
                geometry::CoordinateSystem const& cs) {
  auto stack = std::make_unique<setup::Stack>();

  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  corsika::stack::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

  HEPEnergyType const E0 =
      sqrt(units::static_pow<2>(GetMass(vProjectileType)) + pLab.squaredNorm());
  auto particle = stack->AddParticle(
      std::tuple<Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                 geometry::Point, units::si::TimeType>{vProjectileType, E0, pLab, origin,
                                                       0_ns});

  particle.SetNode(vNodePtr);
  return std::make_tuple(
      std::move(stack), std::make_unique<decltype(setup::StackView{particle})>(particle));
}

TEST_CASE("UrQMD") {
  SECTION("conversion") {
    REQUIRE_THROWS(process::UrQMD::ConvertFromUrQMD(106, 0));
    REQUIRE(process::UrQMD::ConvertFromUrQMD(101, 0) == Code::Pi0);
    REQUIRE(process::UrQMD::ConvertToUrQMD(Code::PiPlus) ==
            std::make_pair<int, int>(101, 2));
  }

  feenableexcept(FE_INVALID);
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("urqmd");
  UrQMD urqmd;

  SECTION("interaction length") {
    auto [env, csPtr, nodePtr] = setupEnvironment(Code::Nitrogen);
    auto const& cs = *csPtr;
    [[maybe_unused]] auto const& env_dummy = env;
    [[maybe_unused]] auto const& node_dummy = nodePtr;

    Code validProjectileCodes[] = {Code::PiPlus,  Code::PiMinus, Code::Proton,
                                   Code::Neutron, Code::KPlus,   Code::KMinus,
                                   Code::K0,      Code::K0Bar,   Code::K0Long};

    for (auto code : validProjectileCodes) {
      auto [stack, view] = setupStack(code, 100_GeV, nodePtr, cs);
      REQUIRE(stack->getEntries() == 1);
      REQUIRE(view->getEntries() == 0);

      // simple check whether the cross-section is non-vanishing
      // only nuclei with available tabluated data so far
      REQUIRE(urqmd.GetInteractionLength(stack->GetNextParticle()) > 1_g / square(1_cm));
    }
  }

  SECTION("nucleus projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setupStack(A, Z, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();
    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) == Z + GetChargeNumber(Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("\"special\" projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] = setupStack(Code::PiPlus, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) ==
            GetChargeNumber(Code::PiPlus) + GetChargeNumber(Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("K0Long projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(Code::Oxygen);
    [[maybe_unused]] auto const& env_dummy = env;      // against warnings
    [[maybe_unused]] auto const& node_dummy = nodePtr; // against warnings

    auto [stackPtr, secViewPtr] = setupStack(Code::K0Long, 400_GeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->getEntries() == 1);
    REQUIRE(secViewPtr->getEntries() == 0);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(*secViewPtr);

    REQUIRE(sumCharge(*secViewPtr) ==
            GetChargeNumber(Code::K0Long) + GetChargeNumber(Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }
}
