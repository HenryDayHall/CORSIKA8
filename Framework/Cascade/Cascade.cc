#include <corsika/cascade/Cascade.h>

using namespace corsika::cascade;

template <typename ProcessList, typename Particle, typename Trajectory, typename Stack>
Cascade<ProcessList, Particle, Trajectory, Stack>::Cascade() {
  //  kkk;
  //  kk;
}

template <typename ProcessList, typename Particle, typename Trajectory, typename Stack>
void Cascade::Init() {
  fStack.Init();
  fProcesseList.Init();
}

template <typename ProcessList, typename Particle, typename Trajectory, typename Stack>
void Cascade::Run() {
  if (!fStack.IsEmpty()) {
    if (!fStack.IsEmpty()) {
      Particle& p = fStack.GetNextParticle();
      Step(p);
    }
    // do cascade equations, which can put new particles on Stack,
    // thus, the double loop
    // DoCascadeEquations(); //
  }
}

template <typename Sequence, typename Trajectory>
void Cascade::Step(Particle& particle) {
  double nextStep = fProcesseList.MinStepLength(particle);
  Trajectory trajectory = fProcesseList.Transport(particle, nextStep);
  sequence.DoContinuous(particle, trajectory);
  sequence.DoDiscrete(particle);
}
