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

auto setupEnvironment(corsika::Code vTargetCode) {
  // setup environment, geometry
  auto env = std::make_unique<corsika::Environment<corsika::IMediumModel>>();
  auto& universe = *(env->GetUniverse());
  const corsika::CoordinateSystem& cs = env->GetCoordinateSystem();

  auto theMedium =
      corsika::Environment<corsika::IMediumModel>::CreateNode<corsika::Sphere>(
          corsika::Point{cs, 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = corsika::HomogeneousMedium<corsika::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      corsika::NuclearComposition(std::vector<corsika::Code>{vTargetCode},
                                  std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  return std::make_tuple(std::move(env), &cs, nodePtr);
}

template <typename TNodeType>
auto setupStack(int vA, int vZ, HEPEnergyType vMomentum, TNodeType* vNodePtr,
                corsika::CoordinateSystem const& cs) {
  auto stack = std::make_unique<setup::Stack>();
  auto constexpr mN = corsika::constants::nucleonMass;

  corsika::Point const origin(cs, {0_m, 0_m, 0_m});
  corsika::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

  HEPEnergyType const E0 = sqrt(static_pow<2>(mN * vA) + pLab.squaredNorm());
  auto particle = stack->AddParticle(
      std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector, corsika::Point,
                 TimeType, unsigned short, unsigned short>{corsika::Code::Nucleus, E0,
                                                           pLab, origin, 0_ns, vA, vZ});

  particle.SetNode(vNodePtr);
  return std::make_tuple(
      std::move(stack),
      std::make_unique<decltype(corsika::SecondaryView(particle))>(particle));
}

template <typename TNodeType>
auto setupStack(corsika::Code vProjectileType, HEPEnergyType vMomentum,
                TNodeType* vNodePtr, corsika::CoordinateSystem const& cs) {
  auto stack = std::make_unique<setup::Stack>();

  corsika::Point const origin(cs, {0_m, 0_m, 0_m});
  corsika::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

  HEPEnergyType const E0 =
      sqrt(static_pow<2>(corsika::mass(vProjectileType)) + pLab.squaredNorm());
  auto particle = stack->AddParticle(
      std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector, corsika::Point,
                 TimeType>{vProjectileType, E0, pLab, origin, 0_ns});

  particle.SetNode(vNodePtr);
  return std::make_tuple(
      std::move(stack),
      std::make_unique<decltype(corsika::SecondaryView(particle))>(particle));
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

  SECTION("cross sections") {
    auto [env, csPtr, nodePtr] = setupEnvironment(corsika::Code::Unknown);
    auto const& cs = *csPtr;
    { [[maybe_unused]] auto const& env_dummy = env; }

    corsika::Code validProjectileCodes[] = {
        corsika::Code::PiPlus,  corsika::Code::PiMinus, corsika::Code::Proton,
        corsika::Code::Neutron, corsika::Code::KPlus,   corsika::Code::KMinus,
        corsika::Code::K0,      corsika::Code::K0Bar,   corsika::Code::K0Long};

    for (auto code : validProjectileCodes) {
      auto [stack, view] = setupStack(code, 100_GeV, nodePtr, cs);
      REQUIRE(stack->GetSize() == 1);

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

  SECTION("nucleon projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(corsika::Code::Oxygen);
    { [[maybe_unused]] auto const& env_dummy = env; }
    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setupStack(A, Z, 400_GeV, nodePtr, *csPtr);
    { [[maybe_unused]] auto const& dummy = stackPtr; }

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();
    [[maybe_unused]] corsika::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(*secViewPtr) == Z + corsika::charge_number(corsika::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("\"special\" projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(corsika::Code::Oxygen);
    { [[maybe_unused]] auto const& env_dummy = env; }
    auto [stackPtr, secViewPtr] =
        setupStack(corsika::Code::PiPlus, 400_GeV, nodePtr, *csPtr);
    { [[maybe_unused]] auto const& dummy = stackPtr; }

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] corsika::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(*secViewPtr) == corsika::charge_number(corsika::Code::PiPlus) +
                                          corsika::charge_number(corsika::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("K0Long projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(corsika::Code::Oxygen);
    { [[maybe_unused]] auto const& env_dummy = env; }
    auto [stackPtr, secViewPtr] =
        setupStack(corsika::Code::K0Long, 400_GeV, nodePtr, *csPtr);
    { [[maybe_unused]] auto const& dummy = stackPtr; }

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] corsika::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(*secViewPtr) == corsika::charge_number(corsika::Code::K0Long) +
                                          corsika::charge_number(corsika::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }
}
