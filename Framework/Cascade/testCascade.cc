#include <corsika/cascade/Cascade.h>
#include <corsika/geometry/LineTrajectory.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::process;
using namespace corsika::units;

#include <iostream>
using namespace std;

class ProcessSplit : public corsika::process::BaseProcess<ProcessSplit> {
public:
  ProcessSplit() {}

  template <typename Particle>
  double MinStepLength(Particle&) const {
    return 0;
  }

  template <typename Particle, typename Trajectory, typename Stack>
  void DoContinuous(Particle& p, Trajectory& t, Stack& s) const {}

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {
    EnergyType E = p.GetEnergy();
    if (E < 1_GeV) {
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
  mutable int fCount = 0;
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
  void DoContinuous(Particle& p, Trajectory& t, Stack& s) const {
    if (!fReport) return;
    static int fCount = 0;
    std::cout << "generation  " << fCount << std::endl;
    int i = 0;
    for (auto& iterP : s) {
      EnergyType E = iterP.GetEnergy();
      std::cout << " particle data: " << i++ << ", id=" << iterP.GetPID()
                << ", E=" << double(E / 1_GeV) << " GeV "
                << " | " << std::endl;
    }
    fCount++;
  }

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {}
  void Init() {}
};

TEST_CASE("Cascade", "[Cascade]") {

  ProcessReport p0(false);
  ProcessSplit p1;
  const auto sequence = p0 + p1;
  corsika::stack::super_stupid::SuperStupidStack stack;

  corsika::cascade::Cascade<corsika::geometry::LineTrajectory, decltype(sequence),
                            decltype(stack)>
      EAS(sequence, stack);

  SECTION("sectionTwo") {
    for (int i = 0; i < 5; ++i) {
      stack.Clear();
      auto particle = stack.NewParticle();
      EnergyType E0 = 100_GeV * pow(10, i);
      particle.SetEnergy(E0);
      EAS.Init();
      EAS.Run();

      cout << "E0=" << E0 / 1_GeV << "GeV, count=" << p1.GetCount() << endl;
    }
  }
}
