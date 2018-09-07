

namespace cascade;

template<typename Sequence, typename Trajectory>
void
Cascade::Cascade() {
  kkk;
  kk;
}


template<typename Sequence, typename Trajectory>
void
Cascade::Init()
{
  fStack.Init();
  fProcesseList.Init();
}


template<typename Sequence, typename Trajectory>
void
Cascade::Run()
{
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


template<typename Sequence, typename Trajectory>
void
Cascade::Step(Particle& particle)
{
  double nextStep = fProcesseList.MinStepLength(particle);
  Trajectory trajectory = fProcesseList.Transport(particle, nextStep);
  sequence.DoContinuous(particle, trajectory);
  sequence.DoDiscrete(particle);
}

