
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <limits>

#include <corsika/environment/Environment.h>

#include <corsika/cascade/Cascade.h>

#include <corsika/process/ProcessSequence.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/NuclearComposition.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
using corsika::setup::Trajectory;

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika::geometry;

#include <iostream>
using namespace std;

environment::Environment MakeDummyEnv() {
  environment::Environment env; // dummy environment
  auto& universe = *(env.GetUniverse());

  auto theMedium = environment::Environment::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel = environment::HomogeneousMedium<environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_g / (1_m * 1_m * 1_m),
      environment::NuclearComposition(
          std::vector<particles::Code>{particles::Code::Proton}, std::vector<float>{1.}));

  universe.AddChild(std::move(theMedium));

  return env;
}

class ProcessSplit : public process::ContinuousProcess<ProcessSplit> {

  int fCount = 0;
  int fCalls = 0;
  HEPEnergyType fEcrit;

public:
  ProcessSplit(HEPEnergyType e)
      : fEcrit(e) {}

  template <typename Particle, typename T>
  LengthType MaxStepLength(Particle&, T&) const {
    return 1_m;
  }

  template <typename Particle, typename T, typename Stack>
  EProcessReturn DoContinuous(Particle& p, T&, Stack&) {
    fCalls++;
    HEPEnergyType E = p.GetEnergy();
    if (E < fEcrit) {
      p.Delete();
      fCount++;
    } else {
      p.SetEnergy(E / 2);
      p.AddSecondary(std::tuple<particles::Code, units::si::HEPEnergyType,
                                corsika::stack::MomentumVector, geometry::Point,
                                units::si::TimeType>{p.GetPID(), E / 2, p.GetMomentum(),
                                                     p.GetPosition(), p.GetTime()});
    }
    return EProcessReturn::eOk;
  }

  void Init() {
    fCount = 0;
    fCalls = 0;
  }

  int GetCount() const { return fCount; }
  int GetCalls() const { return fCalls; }

private:
};

TEST_CASE("Cascade", "[Cascade]") {
  random::RNGManager& rmng = random::RNGManager::GetInstance();
  rmng.RegisterRandomStream("cascade");

  auto env = MakeDummyEnv();
  tracking_line::TrackingLine<setup::Stack, setup::Trajectory> tracking(env);

  stack_inspector::StackInspector<setup::Stack> p0(true);

  const HEPEnergyType Ecrit = 85_MeV;
  ProcessSplit p1(Ecrit);
  auto sequence = p0 << p1;
  setup::Stack stack;

  cascade::Cascade EAS(env, tracking, sequence, stack);
  CoordinateSystem const& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  stack.Clear();
  HEPEnergyType E0 = 100_GeV;
  stack.AddParticle(
      std::tuple<particles::Code, units::si::HEPEnergyType,
                 corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
          particles::Code::Electron, E0,
          corsika::stack::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}),
          Point(rootCS, {0_m, 0_m, 10_km}), 0_ns});
  EAS.Init();
  EAS.Run();

  CHECK(p1.GetCount() == 2048);
  CHECK(p1.GetCalls() == 4095);
}
