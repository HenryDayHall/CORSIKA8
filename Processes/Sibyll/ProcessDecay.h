#ifndef _include_ProcessDecay_h_
#define _include_ProcessDecay_h_

#include <corsika/process/ContinuousProcess.h>

#include <corsika/setup/SetupTrajectory.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/cascade/SibStack.h>

//using namespace corsika::particles;

namespace corsika::process {

  namespace sibyll {


    void setHadronsUnstable(){
      // name? also makes EM particles stable
	
      // loop over all particles in sibyll
      // should be changed to loop over human readable list
      // i.e. corsika::particles::ListOfParticles()
      std::cout << "Sibyll: setting hadrons unstable.." << std::endl;
      // make ALL particles unstable, then set EM stable
      for(auto &p: corsika2sibyll){
	//std::cout << (int)p << std::endl;
	const int sibCode = (int)p;
	// skip unknown and antiparticles
	if( sibCode< 1) continue;
	//std::cout << "Sibyll: Decay: setting " << ConvertFromSibyll( static_cast<SibyllCode> ( sibCode ) ) << " unstable" << std::endl;
	s_csydec_.idb[ sibCode - 1 ] = abs( s_csydec_.idb[ sibCode - 1 ] );
	//std::cout << "decay table value: " << s_csydec_.idb[ sibCode - 1 ] << std::endl;
      }
      // set Leptons and Proton and Neutron stable 
      // use stack to loop over particles
      setup::Stack ds;
      ds.NewParticle().SetPID(corsika::particles::Code::Proton);
      ds.NewParticle().SetPID(corsika::particles::Code::Neutron);
      ds.NewParticle().SetPID(corsika::particles::Code::Electron);
      ds.NewParticle().SetPID(corsika::particles::Code::Positron);
      ds.NewParticle().SetPID(corsika::particles::Code::NuE);
      ds.NewParticle().SetPID(corsika::particles::Code::NuEBar);
      ds.NewParticle().SetPID(corsika::particles::Code::MuMinus);
      ds.NewParticle().SetPID(corsika::particles::Code::MuPlus);
      ds.NewParticle().SetPID(corsika::particles::Code::NuMu);
      ds.NewParticle().SetPID(corsika::particles::Code::NuMuBar);

      for (auto& p : ds) {
	int s_id = process::sibyll::ConvertToSibyllRaw(p.GetPID());
	// set particle stable by setting table value negative
	//	cout << "Sibyll: setting " << p.GetPID() << "(" << s_id << ")"
	//     << " stable in Sibyll .." << endl;
	s_csydec_.idb[ s_id - 1 ] = (-1) * abs(s_csydec_.idb[ s_id - 1 ]);
	p.Delete();
      }	

    }

    
    class ProcessDecay : public corsika::process::BaseProcess<ProcessDecay> {
    public:
      ProcessDecay() {}
      void Init() {
	//setHadronsUnstable();
      }

      void setAllStable(){
	// name? also makes EM particles stable
	
	// loop over all particles in sibyll
	// should be changed to loop over human readable list
	// i.e. corsika::particles::ListOfParticles()
	for(auto &p: corsika2sibyll){
	  //std::cout << (int)p << std::endl;
	  const int sibCode = (int)p;
	  // skip unknown and antiparticles
	  if( sibCode< 1) continue;
	  std::cout << "Sibyll: Decay: setting " << ConvertFromSibyll( static_cast<SibyllCode> ( sibCode ) ) << " stable" << std::endl;
	  s_csydec_.idb[ sibCode - 1 ] = -1 * abs( s_csydec_.idb[ sibCode - 1 ] );
	  std::cout << "decay table value: " << s_csydec_.idb[ sibCode - 1 ] << std::endl;
	}
      }

      friend void setHadronsUnstable();
      
      template <typename Particle>
      double MinStepLength(Particle& p, setup::Trajectory&) const {
	EnergyType E   = p.GetEnergy();
	MassType m     = corsika::particles::GetMass(p.GetPID());
	// env.GetDensity();

	const MassDensityType density = 1.25e-3 * kilogram  / ( 1_cm * 1_cm * 1_cm ); 
    
	const double gamma = E / m / constants::cSquared;

	TimeType t0 = GetLifetime( p.GetPID() );
	cout << "ProcessDecay: MinStep: t0: " << t0 << endl;
	cout << "ProcessDecay: MinStep: gamma: " << gamma << endl;
	cout << "ProcessDecay: MinStep: density: " << density << endl;
	// return as column density
	const double x0 = density * t0 * gamma * constants::c / kilogram * 1_cm * 1_cm;
	cout << "ProcessDecay: MinStep: x0: " << x0 << endl;
	int a = 1;
	const double x = -x0 * log(s_rndm_(a));
	cout << "ProcessDecay: next decay: " << x << endl;	
	return x;
      }
      
      template <typename Particle, typename Stack>
      void DoDiscrete(Particle& p, Stack& s) const {
	SibStack ss;
	ss.Clear();
	// copy particle to sibyll stack
	auto pin = ss.NewParticle();
	pin.SetPID( process::sibyll::ConvertToSibyllRaw( p.GetPID() ) );
	pin.SetEnergy( p.GetEnergy() );
	pin.SetMomentum( p.GetMomentum() );
	// remove original particle from corsika stack
	p.Delete();
	// set all particles/hadrons unstable
	setHadronsUnstable();
	// call sibyll decay
	std::cout << "ProcessDecay: calling Sibyll decay routine.." << std::endl;
	decsib_();
	// print output
	int print_unit = 6;
	sib_list_( print_unit );
	// copy particles from sibyll stack to corsika
	int i = -1;
	for (auto &psib: ss){
	  ++i;
	  // FOR NOW: skip particles that have decayed in Sibyll, move to iterator?
	  if( abs(s_plist_.llist[ i ]) > 100 ) continue;
	  // add to corsika stack
	  //cout << "decay product: " << process::sibyll::ConvertFromSibyll( psib.GetPID() ) << endl;
	  auto pnew = s.NewParticle();
	  pnew.SetEnergy( psib.GetEnergy() );
	  pnew.SetPID( process::sibyll::ConvertFromSibyll( psib.GetPID() ) );	
	  pnew.SetMomentum( psib.GetMomentum() );
	}
	// empty sibyll stack
	ss.Clear();
      }
  
      template <typename Particle, typename Stack>
      EProcessReturn DoContinuous(Particle&, setup::Trajectory&, Stack&) const {
	return EProcessReturn::eOk;
      }
      
    };
  }
}

#endif
