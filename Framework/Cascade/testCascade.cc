
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/cascade/Cascade.h>

#include <corsika/process/ProcessSequence.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/stack/super_stupid/SuperStupidStack.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/geometry/Vector.h>

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

static int fCount = 0;

class ProcessSplit : public corsika::process::ContinuousProcess<ProcessSplit> {
public:
  ProcessSplit() {}

  template <typename Particle, typename T>
  double MaxStepLength(Particle&, T&) const {
    return 1;
  }

  template <typename Particle, typename T, typename Stack>
  void DoContinuous(Particle& p, T&, Stack& s) const {
    EnergyType E = p.GetEnergy();
    if (E < 85_MeV) {
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
  }

  void Init() { fCount = 0; }

  int GetCount() const { return fCount; }

private:
};

TEST_CASE("Cascade", "[Cascade]") {

  corsika::random::RNGManager& rmng = corsika::random::RNGManager::GetInstance();
  ;
  const std::string str_name = "s_rndm";
  rmng.RegisterRandomStream(str_name);

  tracking_line::TrackingLine<setup::Stack> tracking;

  stack_inspector::StackInspector<setup::Stack> p0(true);
  ProcessSplit p1;
  const auto sequence = p0 + p1;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(tracking, sequence, stack);
  CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();

  stack.Clear();
  auto particle = stack.NewParticle();
  EnergyType E0 = 100_GeV;
  particle.SetPID(particles::Code::Electron);
  particle.SetEnergy(E0);
  particle.SetPosition(Point(rootCS, {0_m, 0_m, 10_km}));
  particle.SetMomentum(corsika::stack::super_stupid::MomentumVector(
      rootCS, {0 * newton * second, 0 * newton * second, -1 * newton * second}));
  particle.SetTime(0_ns);
  EAS.Init();
  EAS.Run();

  /*
  SECTION("sectionTwo") {
    for (int i = 0; i < 0; ++i) {
      stack.Clear();
      auto particle = stack.NewParticle();
      EnergyType E0 = 100_GeV * pow(10, i);
      particle.SetEnergy(E0);
      EAS.Init();
      EAS.Run();

      // cout << "Result: E0=" << E0 / 1_GeV << "GeV, count=" << p1.GetCount() << endl;
    }
  }
  */
}
