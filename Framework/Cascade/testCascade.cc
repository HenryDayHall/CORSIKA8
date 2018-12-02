
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

#include <corsika/stack/super_stupid/SuperStupidStack.h>

#include <corsika/geometry/Point.h>
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

class ProcessSplit : public corsika::process::BaseProcess<ProcessSplit> {
public:
  ProcessSplit() {}

  template <typename Particle>
  void MinStepLength(Particle&, setup::Trajectory&) const {}

  template <typename Particle, typename Stack>
  EProcessReturn DoContinuous(Particle&, setup::Trajectory&, Stack&) const {
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {
    EnergyType E = p.GetEnergy();
    if (E < 85_MeV) {
      p.Delete();
      fCount++;
    } else {
      p.SetEnergy(E / 2);
      auto pnew = s.NewParticle();
      // s.Copy(p, pnew);
      pnew.SetEnergy(E / 2);
      pnew.SetPosition(p.GetPosition());
      pnew.SetMomentum(p.GetMomentum());
    }
  }

  void Init() { fCount = 0; }

  int GetCount() const { return fCount; }

private:
};

template <typename Stack>
class NewtonTracking { // naja.. not yet
  typedef typename Stack::ParticleType Particle;

public:
  void Init() {}
  corsika::setup::Trajectory GetTrack(Particle& p) {
    corsika::geometry::Vector<SpeedType::dimension_type> v = p.GetDirection();
    corsika::geometry::Line traj(p.GetPosition(), v);
    {
      CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();
      cout << v.GetComponents(rootCS) << endl;
    }
    return corsika::geometry::Trajectory<corsika::geometry::Line>(traj, 100_ns);
  }
};

TEST_CASE("Cascade", "[Cascade]") {

  NewtonTracking<setup::Stack> tracking;

  stack_inspector::StackInspector<setup::Stack> p0(true);
  ProcessSplit p1;
  const auto sequence = p0 + p1;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(tracking, sequence, stack);

  CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();

  stack.Clear();
  auto particle = stack.NewParticle();
  EnergyType E0 = 100_GeV;
  particle.SetEnergy(E0);
  particle.SetPosition(Point(rootCS, {0_m, 0_m, 10_km}));
  particle.SetMomentum(
      corsika::stack::super_stupid::MomentumVector(rootCS, {0_GeV, 0_GeV, -1_MeV}));
  EAS.Init();
  EAS.Run();

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
}
