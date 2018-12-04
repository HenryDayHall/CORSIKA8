#ifndef _include_ProcessDecay_h_
#define _include_ProcessDecay_h_

#include <corsika/process/ContinuousProcess.h>

#include <corsika/setup/SetupTrajectory.h>
#include <corsika/process/sibyll/ParticleConversion.h>

//using namespace corsika::particles;

namespace corsika::process {

  namespace sibyll {

class ProcessDecay : public corsika::process::BaseProcess<ProcessDecay> {
public:
  ProcessDecay() {}
  void Init() {}
  
  template <typename Particle>
  double MinStepLength(Particle& p, setup::Trajectory&) const {
    EnergyType E   = p.GetEnergy();
    MassType m     = corsika::particles::GetMass(p.GetPID());
    // env.GetDensity();
    const MassDensityType density = 1.25e-3 * kilogram  / ( 1_cm * 1_cm * 1_cm ); 
    
    const double gamma = E / m / constants::cSquared;
    // lifetimes not implemented yet
    TimeType t0;
    switch( p.GetPID() ){
    case corsika::particles::Code::PiPlus :
      t0 = 2.6e-8 * 1_s;
      break;
      
    case corsika::particles::Code::KPlus :
      t0 = 1.e-5 * 1_s;
      break;
      
    default:
      t0 = 1.e8 * 1_s;
      break;
    }
    cout << "ProcessDecay: MinStep: t0: " << t0 << endl;
    cout << "ProcessDecay: MinStep: gamma: " << gamma << endl;
    cout << "ProcessDecay: MinStep: density: " << density << endl;
    // return as column density
    const double x0 = density * t0 * gamma * constants::c / kilogram * 1_cm * 1_cm;
    cout << "ProcessDecay: MinStep: x0: " << x0 << endl;
    return x0;
  }
  
  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {
  }
  
  template <typename Particle, typename Stack>
  EProcessReturn DoContinuous(Particle&, setup::Trajectory&, Stack&) const {
    return EProcessReturn::eOk;
  }

};
  }
}

#endif
