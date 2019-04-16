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

#include <corsika/particles/ParticleProperties.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process::UrQMD;
using namespace corsika::units::si;

TEST_CASE("UrQMD") {
  corsika::random::RNGManager::GetInstance().RegisterRandomStream("UrQMD");
  UrQMD urqmd;

  // setup environment, geometry
  environment::Environment<environment::IMediumModel> env;
  auto& universe = *(env.GetUniverse());
  const geometry::CoordinateSystem& cs = env.GetCoordinateSystem();

  auto theMedium =
      environment::Environment<environment::IMediumModel>::CreateNode<geometry::Sphere>(
          geometry::Point{cs, 0_m, 0_m, 0_m},
          1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_kg / (1_m * 1_m * 1_m),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Oxygen}, std::vector<float>{1.}));

  auto const* nodePtr = theMedium.get();
  universe.AddChild(std::move(theMedium));

  geometry::Point const origin(cs, {0_m, 0_m, 0_m});
  geometry::Vector<units::si::SpeedType::dimension_type> v(cs, 0_m / second, 0_m / second,
                                                           1_m / second);
  geometry::Line line(origin, v);
  geometry::Trajectory<geometry::Line> track(line, 10_s);

  auto constexpr mN = corsika::units::constants::nucleonMass;

  setup::Stack stack;
  const HEPEnergyType P0 = 50_GeV;
  unsigned short constexpr A = 56, Z = 26;
  HEPMomentumType E0 = sqrt(A * A * mN * mN + P0 * P0);
  auto plab = corsika::stack::MomentumVector(cs, {P0, 0_GeV, 0_GeV});
  auto particle =
      stack.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                   corsika::stack::MomentumVector, geometry::Point,
                                   units::si::TimeType, unsigned short, unsigned short>{
          particles::Code::Nucleus, E0, plab, origin, 0_ns, A, Z}); // iron
  particle.SetNode(nodePtr);
  corsika::stack::SecondaryView view(particle);
  auto projectile = view.GetProjectile();

  [[maybe_unused]] const process::EProcessReturn ret = urqmd.DoInteraction(projectile);
}
