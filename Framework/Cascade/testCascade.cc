
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
#include <corsika/geometry/LineTrajectory.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::process;
using namespace corsika::units;

#include <iostream>
using namespace std;

static int fCount = 0;

class ProcessSplit : public corsika::process::BaseProcess<ProcessSplit> {
public:
  ProcessSplit() {}

  template <typename Particle>
  double MinStepLength(Particle&) const {
    return 0;
  }

  template <typename Particle, typename Trajectory, typename Stack>
  EProcessReturn DoContinuous(Particle& p, Trajectory& t, Stack& s) const {
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
      s.NewParticle().SetEnergy(E / 2);
    }
  }

  void Init() { fCount = 0; }

  int GetCount() { return fCount; }

private:
};

class ProcessReport : public corsika::process::BaseProcess<ProcessReport> {
  bool fReport = false;

public:
  ProcessReport(bool v)
      : fReport(v) {}

  template <typename Particle>
  double MinStepLength(Particle&) const {
    return 0;
  }

  template <typename Particle, typename Trajectory, typename Stack>
  EProcessReturn DoContinuous(Particle& p, Trajectory& t, Stack& s) const {
    static int countStep = 0;
    if (!fReport) return EProcessReturn::eOk;
    // std::cout << "generation  " << countStep << std::endl;
    int i = 0;
    EnergyType Etot = 0_GeV;
    for (auto& iterP : s) {
      EnergyType E = iterP.GetEnergy();
      Etot += E;
      /*      std::cout << " particle data: " << i++ << ", id=" << iterP.GetPID()
                << ", E=" << double(E / 1_GeV) << " GeV "
                << " | " << std::endl;
      */
    }
    countStep++;
    // cout << "#=" << countStep << " " << s.GetSize() << " " << Etot/1_GeV << endl;
    cout << countStep << " " << s.GetSize() << " " << Etot / 1_GeV << " " << fCount
         << endl;
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {}
  void Init() {}
};

TEST_CASE("Cascade", "[Cascade]") {

  ProcessReport p0(true);
  ProcessSplit p1;
  const auto sequence = p0 + p1;
  corsika::stack::super_stupid::SuperStupidStack stack;

  corsika::cascade::Cascade<corsika::geometry::LineTrajectory, decltype(sequence),
                            decltype(stack)>
      EAS(sequence, stack);

  stack.Clear();
  auto particle = stack.NewParticle();
  EnergyType E0 = 100_GeV;
  particle.SetEnergy(E0);
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
