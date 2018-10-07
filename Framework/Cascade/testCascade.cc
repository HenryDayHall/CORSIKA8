#include <corsika/cascade/Cascade.h>
#include <corsika/geometry/LineTrajectory.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/stack_inspector/StackInspector.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika;
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
  EProcessReturn DoContinuous(Particle&, Trajectory&, Stack&) const {
    // corsika::utls::ignore(p);
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

TEST_CASE("Cascade", "[Cascade]") {

  stack_inspector::StackInspector<setup::Stack, setup::Trajectory> p0(true);
  ProcessSplit p1;
  const auto sequence = p0 + p1;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(sequence, stack);

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
