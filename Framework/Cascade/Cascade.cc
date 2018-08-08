

namespace cascade;


void
Cascade::Process()
{
  Stack s;
  if (!s.IsEmpty()) {

    s
    
  }
  
}

void
Cascade::Step(auto& sequence, Particle& particle)
{
  double nextStep = sequence.MinStepLength(particle);
  Trajectory trajectory = sequence.Transport(particle, nextStep);
  sequence.DoContinuous(particle, trajectory);
  sequence.DoDiscrete(particle);
}

