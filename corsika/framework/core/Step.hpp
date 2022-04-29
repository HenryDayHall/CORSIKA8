#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

class ParticleState {
    
}

template <typename TParticle, typename TTrajectory>
class Step {
public:
    Step(TParticle const& particle, TTrajectory& track) : particlePreStep_{particle}, track_{track};
    
    
    
private:
    TParticle const& particlePreStep_;
    TTrajectory& track_;
  
};

}
