
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
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/random/RNGManager.h>

#include <corsika/cascade/SibStack.h>
#include <corsika/cascade/sibyll2.3c.h>
#include <corsika/process/sibyll/ParticleConversion.h>

#include <corsika/process/sibyll/ProcessDecay.h>

#include <corsika/units/PhysicalUnits.h>

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::particles;
using namespace corsika::random;

#include <iostream>
#include <typeinfo>
using namespace std;

static int fCount = 0;

class ProcessSplit : public corsika::process::BaseProcess<ProcessSplit> {
public:
  ProcessSplit() {}

  void setTrackedParticlesStable() const {
    /*
      Sibyll is hadronic generator
      only hadrons decay
     */
    // set particles unstable    
    corsika::process::sibyll::setHadronsUnstable();
    // make tracked particles stable
    std::cout << "ProcessSplit: setting tracked hadrons stable.." << std::endl;
    setup::Stack ds;
    ds.NewParticle().SetPID(Code::PiPlus);
    ds.NewParticle().SetPID(Code::PiMinus);
    ds.NewParticle().SetPID(Code::KPlus);
    ds.NewParticle().SetPID(Code::KMinus);
    ds.NewParticle().SetPID(Code::K0Long);
    ds.NewParticle().SetPID(Code::K0Short);
    
    for (auto& p : ds) {
      int s_id = process::sibyll::ConvertToSibyllRaw(p.GetPID());
      // set particle stable by setting table value negative
      //      cout << "ProcessSplit: doDiscrete: setting " << p.GetPID() << "(" << s_id << ")"
      //     << " stable in Sibyll .." << endl;
      s_csydec_.idb[s_id - 1] = (-1) * abs( s_csydec_.idb[s_id - 1] ); 
      p.Delete();
    }

  }
  
  
  template <typename Particle>
  double MinStepLength(Particle& p, setup::Trajectory&) const {

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();
    
    const Code corsikaBeamId = p.GetPID();
    
    // beam particles for sibyll : 1, 2, 3 for p, pi, k
    // read from cross section code table
    int kBeam   =  process::sibyll::GetSibyllXSCode( corsikaBeamId );   

    bool kInteraction = process::sibyll::CanInteract( corsikaBeamId );
    
    /* 
       the target should be defined by the Environment,
       ideally as full particle object so that the four momenta
       and the boosts can be defined..
     */
    // target nuclei: A < 18
    // FOR NOW: assume target is oxygen
    int kTarget = 16;

    // proton mass in units of energy
    const EnergyType proton_mass_en = 0.93827_GeV ;
    
    EnergyType Etot = p.GetEnergy() + kTarget * proton_mass_en;
    super_stupid::MomentumVector Ptot(rootCS, {0.0_newton_second, 0.0_newton_second, 0.0_newton_second});
    // FOR NOW: assume target is at rest
    super_stupid::MomentumVector pTarget(rootCS, {0.0_newton_second, 0.0_newton_second, 0.0_newton_second});
    Ptot += p.GetMomentum();
    Ptot += pTarget;
    // calculate cm. energy
    EnergyType sqs = sqrt( Etot * Etot - Ptot.squaredNorm() * si::constants::cSquared );
    double Ecm = sqs / 1_GeV;
    
    std::cout << "ProcessSplit: " << "MinStep: input en: " << p.GetEnergy() / 1_GeV << endl
      	      << " beam can interact:" << kBeam<< endl
	      << " beam XS code:" << kBeam << endl
      	      << " beam pid:" << p.GetPID() << endl
	      << " target mass number:" << kTarget << std::endl;

    double next_step;
    if(kInteraction){
      
      double prodCrossSection,dummy,dum1,dum2,dum3,dum4;
      double dumdif[3];
      
      if(kTarget==1)
	sib_sigma_hp_(kBeam, Ecm, dum1, dum2, prodCrossSection, dumdif,dum3, dum4 );
      else
	sib_sigma_hnuc_(kBeam, kTarget, Ecm, prodCrossSection, dummy );
    
      std::cout << "ProcessSplit: " << "MinStep: sibyll return: " << prodCrossSection << std::endl;
      CrossSectionType sig = prodCrossSection * 1_mbarn;
      std::cout << "ProcessSplit: " << "MinStep: CrossSection (mb): " << sig / 1_mbarn << std::endl;

      const MassType nucleon_mass = 0.93827_GeV / corsika::units::si::constants::cSquared;
      std::cout << "ProcessSplit: " << "nucleon mass " << nucleon_mass << std::endl;
      // calculate interaction length in medium
      double int_length = kTarget * ( nucleon_mass / 1_g ) / ( sig / 1_cmeter / 1_cmeter );
      // pick random step lenth
      std::cout << "ProcessSplit: " << "interaction length (g/cm2): " << int_length << std::endl;
      // add exponential sampling
      int a = 0;
      next_step = -int_length * log(s_rndm_(a));
    }else
#warning define infinite interaction length? then we can skip the test in DoDiscrete()
      next_step = 1.e8;
    
    /*
      what are the units of the output? slant depth or 3space length?

    */
    std::cout << "ProcessSplit: "
              << "next step (g/cm2): " << next_step << std::endl;
    return next_step;
  }

  template <typename Particle, typename Stack>
  EProcessReturn DoContinuous(Particle&, setup::Trajectory&, Stack&) const {
    // corsika::utls::ignore(p);
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {
    cout << "DoDiscrete: " << p.GetPID() << " interaction? " << process::sibyll::CanInteract( p.GetPID() )  << endl; 
    if( process::sibyll::CanInteract( p.GetPID() ) ){
      cout << "defining coordinates" << endl;
      // coordinate system, get global frame of reference
      CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();

      QuantityVector<length_d> const coordinates{0_m, 0_m, 0_m};
      Point pOrig(rootCS, coordinates);
      
      /* 
	 the target should be defined by the Environment,
	 ideally as full particle object so that the four momenta 
	 and the boosts can be defined..

	 here we need: GetTargetMassNumber() or GetTargetPID()??
	               GetTargetMomentum() (zero in EAS)
      */
      // FOR NOW: set target to proton
      int kTarget = 1; //p.GetPID();

      // proton mass in units of energy
      const EnergyType proton_mass_en = 0.93827_GeV ; //0.93827_GeV / si::constants::cSquared ;

      cout << "defining target momentum.." << endl;
      // FOR NOW: target is always at rest
      const EnergyType Etarget = 0. * 1_GeV + proton_mass_en;      
      const auto pTarget = super_stupid::MomentumVector(rootCS, 0. * 1_GeV / si::constants::c,  0. * 1_GeV / si::constants::c, 0. * 1_GeV / si::constants::c);
      cout << "target momentum (GeV/c): " << pTarget.GetComponents() / 1_GeV * si::constants::c << endl;
      //      const auto pBeam = super_stupid::MomentumVector(rootCS, 0. * 1_GeV / si::constants::c,  0. * 1_GeV / si::constants::c, 0. * 1_GeV / si::constants::c);
      //      cout << "beam momentum: " << pBeam.GetComponents() << endl;
      cout << "beam momentum (GeV/c): " << p.GetMomentum().GetComponents() / 1_GeV * si::constants::c << endl;
      
      // get energy of particle from stack
      /*
	stack is in GeV in lab. frame
	convert to GeV in cm. frame 
	(assuming proton at rest as target AND 
	assuming no pT, i.e. shower frame-z is aligned with hadron-int-frame-z)
      */
      // cout << "defining total energy" << endl;
      // total energy: E_beam + E_target
      // in lab. frame: E_beam + m_target*c**2
      EnergyType E   = p.GetEnergy();
      EnergyType Etot   = E + Etarget;
      // cout << "tot. energy: " << Etot / 1_GeV << endl;
      // cout << "defining total momentum" << endl;
      // total momentum
      super_stupid::MomentumVector Ptot = p.GetMomentum(); // + pTarget;
      // cout << "tot. momentum: " << Ptot.GetComponents() / 1_GeV * si::constants::c << endl;
      // cout << "inv. mass.." << endl;
      // invariant mass, i.e. cm. energy
      EnergyType Ecm = sqrt( Etot * Etot - Ptot.squaredNorm() * si::constants::cSquared ); //sqrt( 2. * E * 0.93827_GeV );
      // cout << "inv. mass: " << Ecm / 1_GeV << endl; 
      // cout << "boost parameters.." << endl;
       /*
	get transformation between Stack-frame and SibStack-frame
	for EAS Stack-frame is lab. frame, could be different for CRMC-mode
	the transformation should be derived from the input momenta
      */      
      //      const double gamma  = ( E + proton_mass * si::constants::cSquared ) / Ecm ;
      // const double gambet =  sqrt( E * E - proton_mass * proton_mass ) / Ecm;
      const double gamma  = Etot / Ecm ;
      const auto gambet = Ptot / (Ecm / si::constants::c );      

      std::cout << "ProcessSplit: " << " DoDiscrete: gamma:" << gamma << endl;
      std::cout << "ProcessSplit: " << " DoDiscrete: gambet:" << gambet.GetComponents() << endl;
      
      int kBeam   = process::sibyll::ConvertToSibyllRaw( p.GetPID() );
    
  
      std::cout << "ProcessSplit: " << " DoDiscrete: E(GeV):" << E / 1_GeV << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;
    if (E < 8.5_GeV || Ecm < 10_GeV ) {
      std::cout << "ProcessSplit: " << " DoDiscrete: dropping particle.." << std::endl;
      p.Delete();
      fCount++;
    } else {
      // Sibyll does not know about units..
      double sqs = Ecm / 1_GeV;
      // running sibyll, filling stack
      sibyll_( kBeam, kTarget, sqs);
      // running decays
      setTrackedParticlesStable();
      decsib_();
      // print final state
      int print_unit = 6;
      sib_list_( print_unit );

      // delete current particle
      p.Delete();

      // add particles from sibyll to stack
      // link to sibyll stack
      SibStack ss;

      // SibStack does not know about momentum yet so we need counter to access momentum array in Sibyll
      int i = -1;
      super_stupid::MomentumVector Ptot_final(rootCS, {0.0_newton_second, 0.0_newton_second, 0.0_newton_second});
      for (auto &psib: ss){
	++i;
	// skip particles that have decayed in Sibyll
	if( abs(s_plist_.llist[ i ]) > 100 ) continue;
	
	//transform energy to lab. frame, primitve
	// compute beta_vec * p_vec
	// arbitrary Lorentz transformation based on sibyll routines
	const auto gammaBetaComponents = gambet.GetComponents();
	const auto pSibyllComponents = psib.GetMomentum().GetComponents();	
	EnergyType en_lab = 0. * 1_GeV;	
	MomentumType p_lab_components[3];
	en_lab =  psib.GetEnergy() * gamma;
	EnergyType pnorm = 0. * 1_GeV;
	for(int j=0; j<3; ++j)
	  pnorm += ( pSibyllComponents[j] * gammaBetaComponents[j] * si::constants::c ) / ( gamma + 1.);
	pnorm += psib.GetEnergy();
	
	for(int j=0; j<3; ++j){
	  p_lab_components[j] = pSibyllComponents[j] - (-1) * pnorm * gammaBetaComponents[j] /  si::constants::c;
	  // cout << "p:" << j << " pSib (GeV/c): " << pSibyllComponents[j] / 1_GeV * si::constants::c
	  //      << " gb: " << gammaBetaComponents[j] << endl;
	  en_lab -= (-1) * pSibyllComponents[j] * gammaBetaComponents[j] * si::constants::c;
	}
	//	const EnergyType en_lab = psib.GetEnergy()*gamma + gambet * psib.GetMomentum() * si::constants::c );
	// cout << " en cm  (GeV): " << psib.GetEnergy() / 1_GeV << endl
	//      << " en lab (GeV): " << en_lab / 1_GeV << endl;
	// cout << " pz cm  (GeV/c): " << psib.GetMomentum().GetComponents()[2] / 1_GeV * si::constants::c << endl
	//      << " pz lab (GeV/c): " << p_lab_components[2] / 1_GeV * si::constants::c << endl;
	
	// add to corsika stack
	auto pnew = s.NewParticle();
	pnew.SetEnergy( en_lab );
	pnew.SetPID( process::sibyll::ConvertFromSibyll( psib.GetPID() ) );
	//cout << "momentum sib (cm): " << psib.GetMomentum().GetComponents() / 1_GeV * si::constants::c << endl;
	
	corsika::geometry::QuantityVector<momentum_d> p_lab_c{ p_lab_components[0],
							       p_lab_components[1],
							       p_lab_components[2]};
	pnew.SetMomentum( super_stupid::MomentumVector( rootCS, p_lab_c) );
	//cout << "momentum sib (lab): " << pnew.GetMomentum().GetComponents() / 1_GeV * si::constants::c << endl;
	//cout << "s_cm  (GeV2): " << (psib.GetEnergy() * psib.GetEnergy() - psib.GetMomentum().squaredNorm() * si::constants::cSquared ) / 1_GeV / 1_GeV << endl;
	//cout << "s_lab (GeV2): " << (pnew.GetEnergy() * pnew.GetEnergy() - pnew.GetMomentum().squaredNorm() * si::constants::cSquared ) / 1_GeV / 1_GeV << endl;
	Ptot_final += pnew.GetMomentum();
      }
      //cout << "tot. momentum final (GeV/c): " << Ptot_final.GetComponents() / 1_GeV * si::constants::c << endl;
    }
    }else
      p.Delete();
  }

  void Init() {
    fCount = 0;

    // define reference frame? --> defines boosts between corsika stack and model stack.

    // initialize random numbers for sibyll
    // FOR NOW USE SIBYLL INTERNAL !!!
    //    rnd_ini_();

    corsika::random::RNGManager& rmng = corsika::random::RNGManager::GetInstance();
    ;
    const std::string str_name = "s_rndm";
    rmng.RegisterRandomStream(str_name);

    // //    corsika::random::RNG srng;
    // auto srng = rmng.GetRandomStream("s_rndm");

    // test random number generator
    std::cout << "ProcessSplit: "
              << " test sequence of random numbers." << std::endl;
    int a = 0;
    for (int i = 0; i < 8; ++i) std::cout << i << " " << s_rndm_(a) << std::endl;

    // initialize Sibyll
    sibyll_ini_();

    setTrackedParticlesStable();
  }

  
  int GetCount() { return fCount; }

private:
};

double s_rndm_(int&) {
  static corsika::random::RNG& rmng =
      corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  ;
  return rmng() / (double)rmng.max();
}


int main() {

  // coordinate system, get global frame of reference
  CoordinateSystem& rootCS = RootCoordinateSystem::GetInstance().GetRootCS();
  
  QuantityVector<length_d> const coordinates{0_m, 0_m, 0_m};
  Point pOrig(rootCS, coordinates);    

  tracking_line::TrackingLine<setup::Stack> tracking;
  stack_inspector::StackInspector<setup::Stack> p0(true);

  ProcessSplit p1;
  corsika::process::sibyll::ProcessDecay p2;
  const auto sequence = p0 + p1 + p2;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(tracking, sequence, stack);

  stack.Clear();
  auto particle = stack.NewParticle();
  EnergyType E0 = 100_GeV;
  MomentumType P0 = sqrt( E0*E0 - 0.93827_GeV * 0.93827_GeV ) / si::constants::c;
  auto plab = super_stupid::MomentumVector(rootCS,
					   0. * 1_GeV / si::constants::c,
					   0. * 1_GeV / si::constants::c,
					   P0);
  particle.SetEnergy(E0);
  particle.SetMomentum(plab);
  particle.SetPID( Code::Proton );

  EAS.Init();
  EAS.Run();
  cout << "Result: E0=" << E0 / 1_GeV << "GeV, count=" << p1.GetCount() << endl;
}
