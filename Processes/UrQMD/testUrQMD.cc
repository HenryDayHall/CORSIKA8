/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
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

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::UrQMD;
using namespace corsika::units::si;

template <typename TStack>
auto sumCharge(TStack& stack) {
  int totalCharge = 0;
  int count = 0;
  for (auto& p : stack) {
    std::cout << count++ << " ";
    totalCharge += particles::GetChargeNumber(p.GetPID());
    std::cout << p.GetPID() << " " << particles::GetChargeNumber(p.GetPID()) << ' '
              << p.GetMomentum().GetComponents() << std::endl;
  }

  std::cout << count << " particles on stack" << std::endl;

  return totalCharge;
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
      environment::NuclearComposition(
          std::vector<particles::Code>{vTargetCode}, std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));
  
  return std::make_tuple(std::move(env), &cs, nodePtr);
}

template <typename TNodeType>
auto setupStack(int vA, int vZ, HEPEnergyType vMomentum, TNodeType* vNodePtr, geometry::CoordinateSystem const& cs) {
    auto stack = std::make_unique<setup::Stack>();
    auto constexpr mN = corsika::units::constants::nucleonMass;
    
    geometry::Point const origin(cs, {0_m, 0_m, 0_m});
    corsika::stack::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});
    
    HEPEnergyType const E0 = sqrt(units::si::detail::static_pow<2>(mN*vA) + pLab.squaredNorm());
    auto particle =
        stack->AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                     corsika::stack::MomentumVector, geometry::Point,
                                     units::si::TimeType, unsigned short, unsigned short>{
            particles::Code::Nucleus, E0, pLab, origin, 0_ns, vA, vZ});

    particle.SetNode(vNodePtr);
    return std::make_tuple(std::move(stack), std::make_unique<decltype(corsika::stack::SecondaryView(particle))>(particle));
}

template <typename TNodeType>
auto setupStack(particles::Code vProjectileType, HEPEnergyType vMomentum, TNodeType* vNodePtr, geometry::CoordinateSystem const& cs) {
    auto stack = std::make_unique<setup::Stack>();
    
    geometry::Point const origin(cs, {0_m, 0_m, 0_m});
    corsika::stack::MomentumVector const pLab(cs, {vMomentum, 0_GeV, 0_GeV});
    
    HEPEnergyType const E0 = sqrt(units::si::detail::static_pow<2>(particles::GetMass(vProjectileType)) + pLab.squaredNorm());
    auto particle =
        stack->AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                     corsika::stack::MomentumVector, geometry::Point,
                                     units::si::TimeType>{
            vProjectileType, E0, pLab, origin, 0_ns});

    particle.SetNode(vNodePtr);
    return std::make_tuple(std::move(stack), std::make_unique<decltype(corsika::stack::SecondaryView(particle))>(particle));
}

//~ int main() {
TEST_CASE("UrQMD") {
  feenableexcept(FE_INVALID);
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("UrQMD");
  UrQMD urqmd;
  
  SECTION("cross sections") {
      auto [env, csPtr, nodePtr] = setupEnvironment(particles::Code::Invalid);
      auto const& cs = *csPtr;
      
      particles::Code validProjectileCodes[] = {particles::Code::PiPlus, particles::Code::PiMinus,
          particles::Code::Proton, particles::Code::Neutron};
          
      for (auto code: validProjectileCodes) {
        auto [stack, view] = setupStack(code, 100_GeV, nodePtr, cs);
        REQUIRE( stack->GetSize() == 1);
      
        // simple check whether the cross-section is non-vanishing
        REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Proton) / 1_mb > 0);
        REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Nitrogen) / 1_mb > 0);
        REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Oxygen) / 1_mb > 0);
        REQUIRE(urqmd.GetCrossSection(view->GetProjectile(), particles::Code::Nitrogen) / 1_mb > 0);
      }
  }

  SECTION("nucleon projectile") {
    unsigned short constexpr A = 16, Z = 8;

    [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    REQUIRE(sumCharge(stack) == Z + particles::GetChargeNumber(particles::Code::Oxygen));
  }

  //~ SECTION("\"special\" projectile") {


    //~ [[maybe_unused]] process::EProcessReturn const ret = urqmd.DoInteraction(projectile);

    //~ REQUIRE(sumCharge(stack) == particles::GetChargeNumber(particles::Code::PiPlus) +
                                    //~ particles::GetChargeNumber(particles::Code::Oxygen));
  //~ }
}
