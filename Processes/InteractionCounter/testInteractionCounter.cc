/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */


#include <corsika/process/interaction_counter/InteractionCounter.h>

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

auto setupEnvironment(particles::Code target_code) {
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
      environment::NuclearComposition(std::vector<particles::Code>{target_code},
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

TEST_CASE("InteractionCounter") {
  DummyProcess d;
  InteractionCounter countedProcess(d);

  SECTION("GetInteractionLength") {
    REQUIRE(countedProcess.GetInteractionLength(nullptr) == 100_g / 1_cm / 1_cm);
  }

  auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Oxygen);
  [[maybe_unused]] auto& env_dummy = env;
  
  SECTION("DoInteraction nucleus") {
    unsigned short constexpr A = 14, Z = 7;
    auto [stackPtr, secViewPtr] = setupStack(A, Z, 105_TeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->GetSize() == 1);
    REQUIRE(secViewPtr->GetSize() == 0);

    auto projectile = secViewPtr->GetProjectile();
    auto const ret = countedProcess.DoInteraction(projectile);
    REQUIRE(ret == nullptr);

    auto const& h = countedProcess.GetHistogram().labHists().second.at(1'000'070'140);
    REQUIRE(h.at(50) == 1);
    REQUIRE(std::accumulate(h.cbegin(), h.cend(), 0) == 1);

    auto const& h2 = countedProcess.GetHistogram().CMSHists().second.at(1'000'070'140);
    REQUIRE(h2.at(32) == 1); // bin 1.584 .. 1.995 TeV √s
    REQUIRE(std::accumulate(h2.cbegin(), h2.cend(), 0) == 1);
  }

  SECTION("DoInteraction Lambda") {
    auto constexpr code = particles::Code::Lambda0;
    auto constexpr codeInt = static_cast<particles::CodeIntType>(code);
    auto [stackPtr, secViewPtr] = setupStack(code, 105_TeV, nodePtr, *csPtr);
    REQUIRE(stackPtr->GetSize() == 1);
    REQUIRE(secViewPtr->GetSize() == 0);

    auto projectile = secViewPtr->GetProjectile();
    auto const ret = countedProcess.DoInteraction(projectile);
    REQUIRE(ret == nullptr);

    auto const& h = countedProcess.GetHistogram().labHists().first;
    REQUIRE(h.at(codeInt, 50) == 1);
    REQUIRE(std::accumulate(h.cbegin(), h.cend(), 0) == 1);

    auto const& h2 = countedProcess.GetHistogram().CMSHists().first;
    REQUIRE(h2.at(codeInt, 32) == 1); // bin 1.584 .. 1.995 TeV √s
    REQUIRE(std::accumulate(h2.cbegin(), h2.cend(), 0) == 1);
  }
}
