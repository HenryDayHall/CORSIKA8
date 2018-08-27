#include <Sibyll/Particle.h>

using namespace processes;

const std::map<sibyll::PID, ParticleProperties::InternalParticleCode>
processes::sibyll::Sibyll2Corsika = {
  {sibyll::PID::E_MINUS, InternalParticleCode::Electron},
};

