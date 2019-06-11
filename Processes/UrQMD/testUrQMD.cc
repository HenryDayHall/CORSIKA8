/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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

auto setupEnvironment(particles::Code vTargetCode) {
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
      environment::NuclearComposition(std::vector<particles::Code>{vTargetCode},
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

  HEPEnergyType const E0 =
      sqrt(units::si::detail::static_pow<2>(mN * vA) + pLab.squaredNorm());
  auto particle =
      stack->AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                    corsika::stack::MomentumVector, geometry::Point,
                                    units::si::TimeType, unsigned short, unsigned short>{
          particles::Code::Nucleus, E0, pLab, origin, 0_ns, vA, vZ});

  particle.SetNode(vNodePtr);
  return std::make_tuple(
      std::move(stack),
      std::make_unique<decltype(corsika::stack::SecondaryView(particle))>(particle));
}

template <typename TNodeType>
auto setupStack(particles::Code vProjectileType, HEPEnergyType vMomentum,
                TNodeType* vNodePtr, geometry::CoordinateSystem const& cs) {
  auto stack = std::make_unique<setup::Stack>();

  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  corsika::stack::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});

  HEPEnergyType const E0 =
      sqrt(units::si::detail::static_pow<2>(particles::GetMass(vProjectileType)) +
           pLab.squaredNorm());
  auto particle = stack->AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
          vProjectileType, E0, pLab, origin, 0_ns});

  particle.SetNode(vNodePtr);
  return std::make_tuple(
      std::move(stack),
      std::make_unique<decltype(corsika::stack::SecondaryView(particle))>(particle));
}

TEST_CASE("UrQMD") {
  SECTION("conversion") {
    REQUIRE_THROWS(process::UrQMD::ConvertFromUrQMD(106, 0));
    REQUIRE(process::UrQMD::ConvertFromUrQMD(101, 0) == particles::Code::Pi0);
    REQUIRE(process::UrQMD::ConvertToUrQMD(particles::Code::PiPlus) ==
            std::make_pair<int, int>(101, 2));
  }

  feenableexcept(FE_INVALID);
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("UrQMD");
  UrQMD urqmd;

  SECTION("cross sections") {
    auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Unknown);
    auto const& cs = *csPtr;

    particles::Code validProjectileCodes[] = {
        particles::Code::PiPlus,  particles::Code::PiMinus, particles::Code::Proton,
        particles::Code::Neutron, particles::Code::KPlus,   particles::Code::KMinus,
        particles::Code::K0,      particles::Code::K0Bar,   particles::Code::K0Long};

    for (auto code : validProjectileCodes) {
      auto [stack, view] = setupStack(code, 100_GeV, nodePtr, cs);
      REQUIRE(stack->GetSize() == 1);

      // simple check whether the cross-section is non-vanishing
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Proton) /
                  1_mb >
              0);
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Nitrogen) /
                  1_mb >
              0);
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Oxygen) /
                  1_mb >
              0);
      REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Argon) /
                  1_mb >
              0);
    }
  }

  SECTION("nucleon projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Oxygen);
    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setupStack(A, Z, 400_GeV, nodePtr, *csPtr);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();
    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(*secViewPtr) ==
            Z + particles::GetChargeNumber(particles::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("\"special\" projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Oxygen);
    auto [stackPtr, secViewPtr] =
        setupStack(particles::Code::PiPlus, 400_GeV, nodePtr, *csPtr);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(*secViewPtr) ==
            particles::GetChargeNumber(particles::Code::PiPlus) +
                particles::GetChargeNumber(particles::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }

  SECTION("K0Long projectile") {
    auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Oxygen);
    auto [stackPtr, secViewPtr] =
        setupStack(particles::Code::K0Long, 400_GeV, nodePtr, *csPtr);

    // must be assigned to variable, cannot be used as rvalue?!
    auto projectile = secViewPtr->GetProjectile();
    auto const projectileMomentum = projectile.GetMomentum();

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(*secViewPtr) ==
            particles::GetChargeNumber(particles::Code::K0Long) +
                particles::GetChargeNumber(particles::Code::Oxygen));

    auto const secMomSum =
        sumMomentum(*secViewPtr, projectileMomentum.GetCoordinateSystem());
    REQUIRE((secMomSum - projectileMomentum).norm() / projectileMomentum.norm() ==
            Approx(0).margin(1e-2));
  }
}
