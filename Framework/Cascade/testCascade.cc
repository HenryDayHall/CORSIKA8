/**
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

#include <corsika/stack/super_stupid/SuperStupidStack.h>

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
using namespace corsika::geometry;

#include <iostream>
using namespace std;
using namespace corsika::units::si;

corsika::environment::Environment MakeDummyEnv() {
  corsika::environment::Environment env; // dummy environment
  auto& universe = *(env.GetUniverse());

  auto theMedium = corsika::environment::Environment::CreateNode<Sphere>(
      Point{env.GetCoordinateSystem(), 0_m, 0_m, 0_m},
      1_km * std::numeric_limits<double>::infinity());

  using MyHomogeneousModel =
      corsika::environment::HomogeneousMedium<corsika::environment::IMediumModel>;
  theMedium->SetModelProperties<MyHomogeneousModel>(
      1_g / (1_m * 1_m * 1_m),
      corsika::environment::NuclearComposition(
          std::vector<corsika::particles::Code>{corsika::particles::Code::Proton},
          std::vector<float>{1.}));

  universe.AddChild(std::move(theMedium));

  return env;
}

class ProcessSplit : public corsika::process::ContinuousProcess<ProcessSplit> {

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
  EProcessReturn DoContinuous(Particle& p, T&, Stack& s) {
    fCalls++;
    HEPEnergyType E = p.GetEnergy();
    if (E < fEcrit) {
      p.Delete();
      fCount++;
    } else {
      p.SetEnergy(E / 2);
      auto pnew = s.NewParticle();
      // s.Copy(p, pnew); fix that .... todo
      pnew.SetPID(p.GetPID());
      pnew.SetTime(p.GetTime());
      pnew.SetEnergy(E / 2);
      pnew.SetPosition(p.GetPosition());
      pnew.SetMomentum(p.GetMomentum());
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
  corsika::random::RNGManager& rmng = corsika::random::RNGManager::GetInstance();
  rmng.RegisterRandomStream("cascade");

  auto env = MakeDummyEnv();
  tracking_line::TrackingLine<setup::Stack> tracking(env);

  stack_inspector::StackInspector<setup::Stack> p0(true);

  const HEPEnergyType Ecrit = 85_MeV;
  ProcessSplit p1(Ecrit);
  auto sequence = p0 << p1;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(env, tracking, sequence, stack);
  CoordinateSystem const& rootCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  stack.Clear();
  auto particle = stack.NewParticle();
  HEPEnergyType E0 = 100_GeV;
  particle.SetPID(particles::Code::Electron);
  particle.SetEnergy(E0);
  particle.SetPosition(Point(rootCS, {0_m, 0_m, 10_km}));
  particle.SetMomentum(
      corsika::stack::super_stupid::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_GeV}));
  particle.SetTime(0_ns);
  EAS.Init();
  EAS.Run();

  CHECK(p1.GetCount() == 2048);
  CHECK(p1.GetCalls() == 4095);

  /*
  SECTION("sectionTwo") {
    for (int i = 0; i < 0; ++i) {
      stack.Clear();
      auto particle = stack.NewParticle();
      HEPEnergyType E0 = 100_GeV * pow(10, i);
      particle.SetEnergy(E0);
      EAS.Init();
      EAS.Run();

      // cout << "Result: E0=" << E0 / 1_GeV << "GeV, count=" << p1.GetCount() << endl;
    }
  }
  */
}
