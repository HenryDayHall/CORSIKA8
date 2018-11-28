
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
#include <corsika/geometry/LineTrajectory.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/process/stack_inspector/StackInspector.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <corsika/random/RNGManager.h>
#include <corsika/cascade/sibyll2.3c.h>

//#include <corsika/units/PhysicalConstants.h>
#include <corsika/units/PhysicalUnits.h>
using namespace corsika;
using namespace corsika::process;
using namespace corsika::units;
using namespace corsika::particles;
using namespace corsika::random;

#include <iostream>
using namespace std;

static int fCount = 0;

class ProcessSplit : public corsika::process::BaseProcess<ProcessSplit> {
public:
  ProcessSplit() {}

  template <typename Particle>
  double MinStepLength(Particle& p) const {
    // beam particles for sibyll : 1, 2, 3 for p, pi, k
    int kBeam   = 1;

    /* 
       the target should be defined by the Environment,
       ideally as full particle object so that the four momenta 
       and the boosts can be defined..
     */
    // target nuclei: A < 18
    // FOR NOW: assume target is oxygen
    int kTarget = 16;
    double beamEnergy =  p.GetEnergy() / 1_GeV; 
    std::cout << "ProcessSplit: " << "MinStep: en: " << beamEnergy << " pid:" << kBeam << std::endl;
    double prodCrossSection,dummy;
    
    sib_sigma_hnuc_(kBeam, kTarget, beamEnergy, prodCrossSection, dummy );
    
    std::cout << "ProcessSplit: " << "MinStep: sibyll return: " << prodCrossSection << std::endl;
    CrossSectionType sig = prodCrossSection  / 1000. * barn;
    std::cout << "ProcessSplit: " << "MinStep: CrossSection (mb): " << sig / 1_mbarn << std::endl;

    const MassType nucleon_mass = 0.93827_GeV / corsika::units::si::constants::cSquared;
    std::cout << "ProcessSplit: " << "nucleon mass " << nucleon_mass << std::endl;
    // calculate interaction length in medium
    double int_length = kTarget * ( nucleon_mass / 1_g ) / ( sig / 1_cmeter / 1_cmeter );
    // pick random step lenth
    std::cout << "ProcessSplit: " << "interaction length (g/cm2): " << int_length << std::endl;
    // add exponential sampling
    int a = 0;
    const double next_step = -int_length * log(s_rndm_(a));
    /*
      what are the units of the output? slant depth or 3space length?
      
    */
    std::cout << "ProcessSplit: " << "next step (g/cm2): " << next_step << std::endl;
    return next_step;
  }

  template <typename Particle, typename Trajectory, typename Stack>
  EProcessReturn DoContinuous(Particle&, Trajectory&, Stack&) const {
    // corsika::utls::ignore(p);
    return EProcessReturn::eOk;
  }

  template <typename Particle, typename Stack>
  void DoDiscrete(Particle& p, Stack& s) const {
    // get energy of particle from stack
    /*
      stack is in GeV in lab. frame
      convert to GeV in cm. frame 
      (assuming proton at rest as target AND 
      assuming no pT, i.e. shower frame-z is aligned with hadron-int-frame-z)
    */
    EnergyType E   = p.GetEnergy();
    EnergyType Ecm = sqrt( 2. * E * 0.93827_GeV );
    // FOR NOW: set beam to proton
    int kBeam   = 13; //p.GetPID();
    /* 
       the target should be defined by the Environment,
       ideally as full particle object so that the four momenta 
       and the boosts can be defined..
     */
    // FOR NOW: set target to proton
    int kTarget = 1; //p.GetPID();
    std::cout << "ProcessSplit: " << " DoDiscrete: E(GeV):" << E / 1_GeV << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;
    if (E < 8.5_GeV || Ecm < 10_GeV ) {
      std::cout << "ProcessSplit: " << " DoDiscrete: dropping particle.." << std::endl;
      p.Delete();
      fCount++;
    } else {
      // Sibyll does not know about units..
      double sqs = Ecm / 1_GeV;
      // running sibyll
      sibyll_( kBeam, kTarget, sqs);
      // running decays
      //decsib_();
      // print final state
      int print_unit = 6;
      sib_list_( print_unit );
      
      // delete current particle
      p.Delete();

      const EnergyType proton_mass = 0.93827_GeV;
      const double gamma  = ( E + proton_mass ) / ( Ecm );
      const double gambet =  sqrt( E * E - proton_mass * proton_mass ) / Ecm;
      
      // add particles from sibyll to stack
      for(int i=0; i<s_plist_.np; ++i){
	//transform to lab. frame, primitve
	const double en_lab = gambet * s_plist_.p[2][i] + gamma * s_plist_.p[3][i];
	// add to corsika stack
	s.NewParticle().SetEnergy( en_lab * 1_GeV );
      }
    }
  }

  void Init() //{ fCount = 0; }
  {
    fCount = 0;

    // define reference frame? --> defines boosts between corsika stack and model stack.
    
    // initialize random numbers for sibyll
    // FOR NOW USE SIBYLL INTERNAL !!!
    rnd_ini_();
    
    // corsika::random::RNGManager rmng;
    // const std::string str_name = "s_rndm";
    // rmng.RegisterRandomStream(str_name);

    // //    corsika::random::RNG srng;
    // auto srng = rmng.GetRandomStream("s_rndm");

    // test random number generator
    std::cout << "ProcessSplit: " << " test sequence of random numbers."  << std::endl;
    int a = 0;
    for(int i=0; i<5; ++i)
      std::cout << i << " " << s_rndm_(a) << std::endl;
    
    //initialize Sibyll
    sibyll_ini_();

    // set particles stable / unstable
  }
  
  int GetCount() { return fCount; }

private:
};

int main(){

  stack_inspector::StackInspector<setup::Stack, setup::Trajectory> p0(true);
  ProcessSplit p1;
  const auto sequence = p0 + p1;
  setup::Stack stack;

  corsika::cascade::Cascade EAS(sequence, stack);

  stack.Clear();
  auto particle = stack.NewParticle();
  EnergyType E0 = 100_GeV;
  particle.SetEnergy(E0);
  particle.SetPID( Code::Proton );
  EAS.Init();
  EAS.Run();
  cout << "Result: E0=" << E0 / 1_GeV << "GeV, count=" << p1.GetCount() << endl;
  
}
