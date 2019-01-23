
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_sibyll_nuclearinteraction_h_
#define _corsika_process_sibyll_nuclearinteraction_h_

#include <corsika/process/InteractionProcess.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>
#include <corsika/process/sibyll/sibyll2.3c.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

namespace corsika::process::sibyll {

  class NuclearInteraction : public corsika::process::InteractionProcess<NuclearInteraction> {

    int fCount = 0;
    int fNucCount = 0;

  public:
    NuclearInteraction(corsika::environment::Environment const& env)
        : fEnvironment(env) {}
    ~NuclearInteraction() {
      std::cout << "Sibyll::NuclearInteraction n=" << fCount << " Nnuc=" << fNucCount
                << std::endl;
    }

    void Init() {

      using corsika::random::RNGManager;
      using std::cout;
      using std::endl;

      // initialize hadronic interaction module
      sibyll_ini_();
      
      // initialize nuclib
      nuc_nuc_ini_();
    }

    std::tuple<corsika::units::si::CrossSectionType, int> GetCrossSection(
        const corsika::particles::Code BeamId, const corsika::particles::Code TargetId,
        const corsika::units::si::HEPEnergyType CoMenergy) {
      using namespace corsika::units::si;
      double sigProd, dummy, dum1, dum2, dum3, dum4;
      double dumdif[3];

      corsika::particles::Code BeamIdToUse;
      if(corsika::particles::IsNucleus(BeamId)){

	// TODO: use nuclib to calc. nuclear cross sections
	// FOR NOW: use proton cross section for nuclei
	BeamIdToUse = corsika::particles::Proton::GetCode();
	std::cout << "WARNING: replacing beam nucleus with proton!" << std::endl;
      } else {
	BeamIdToUse = BeamId;
      }
      
      const int iBeam = process::sibyll::GetSibyllXSCode(BeamIdToUse);
      const double dEcm = CoMenergy / 1_GeV;
      if (corsika::particles::IsNucleus(TargetId)) {
	const int iTarget = corsika::particles::GetNucleusA(TargetId);
	if (iTarget > 18 || iTarget == 0)
	  throw std::runtime_error(
				   "Sibyll target outside range. Only nuclei with A<18 are allowed.");
	sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy);
	return std::make_tuple(sigProd * 1_mbarn, iTarget);
      } else if (TargetId == corsika::particles::Proton::GetCode()) {
	sib_sigma_hp_(iBeam, dEcm, dum1, dum2, sigProd, dumdif, dum3, dum4);
	return std::make_tuple(sigProd * 1_mbarn, 1);
      } else {
	throw std::runtime_error("GetCrossSection: no interaction in sibyll possible");
	// no interaction in sibyll possible, return infinite cross section? or throw?
	sigProd = std::numeric_limits<double>::infinity();
	return std::make_tuple(sigProd * 1_mbarn, 0);
      }
    }

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle const& p, Track&) {

      using namespace corsika::units;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      // coordinate system, get global frame of reference
      CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      const particles::Code corsikaBeamId = p.GetPID();

      if(!corsika::particles::IsNucleus(corsikaBeamId)){
	// no nuclear interaction
	return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
      
      // beam particles for sibyll : 1, 2, 3 for p, pi, k
      // read from cross section code table
      const HEPMassType nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
					      corsika::particles::Neutron::GetMass());

      // FOR NOW: assume target is at rest
      MomentumVector pTarget(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});

      // total momentum and energy
      HEPEnergyType Elab = p.GetEnergy() + nucleon_mass;
      MomentumVector pTotLab(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      pTotLab += p.GetMomentum();
      pTotLab += pTarget;
      auto const pTotLabNorm = pTotLab.norm();
      // calculate cm. energy
      const HEPEnergyType ECoM = sqrt(
				      (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

      std::cout << "NuclearInteraction: LambdaInt: \n"
		<< " input energy: " << p.GetEnergy() / 1_GeV << endl
		<< " beam pid:" << p.GetPID() << endl;

      if ( Elab >= 8.5_GeV && ECoM >= 10_GeV) {

	// get target from environment
	/*
	  the target should be defined by the Environment,
	  ideally as full particle object so that the four momenta
	  and the boosts can be defined..
	*/
	const auto currentNode =
	  fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
	const auto mediumComposition =
	  currentNode->GetModelProperties().GetNuclearComposition();
	// determine average interaction length
	// weighted sum
	int i = -1;
	double avgTargetMassNumber = 0.;
	si::CrossSectionType weightedProdCrossSection = 0_mbarn;
	// get weights of components from environment/medium
	const auto w = mediumComposition.GetFractions();
	// loop over components in medium
	for (auto const targetId : mediumComposition.GetComponents()) {
	  i++;
	  cout << "NuclearInteraction: get interaction length for target: " << targetId << endl;

	  auto const [productionCrossSection, numberOfNucleons] =
	    GetCrossSection(corsikaBeamId, targetId, ECoM);

	  std::cout << "NuclearInteraction: "
		    << " IntLength: sibyll return (mb): "
		    << productionCrossSection / 1_mbarn << std::endl;
	  weightedProdCrossSection += w[i] * productionCrossSection;
	  avgTargetMassNumber += w[i] * numberOfNucleons;
	}
	cout << "NuclearInteraction: "
	     << "IntLength: weighted CrossSection (mb): "
	     << weightedProdCrossSection / 1_mbarn << endl;

	// calculate interaction length in medium
	GrammageType const int_length =
	  avgTargetMassNumber * corsika::units::constants::u / weightedProdCrossSection;
	std::cout << "NuclearInteraction: "
		  << "interaction length (g/cm2): "
		  << int_length / (0.001_kg) * 1_cm * 1_cm << std::endl;

	return int_length;
      } else {
	return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
    }
   
    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle& p, Stack& s) {

      using namespace corsika::units;
      using namespace corsika::utl;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      const auto corsikaBeamId = p.GetPID();
      cout << "NuclearInteraction: DoInteraction: called with:" << corsikaBeamId
	   << endl;
	      
      if(!IsNucleus(corsikaBeamId))
	return process::EProcessReturn::eOk;

      const CoordinateSystem& rootCS =
	RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
      
      // position and time of interaction, not used in Sibyll
      Point pOrig = p.GetPosition();
      TimeType tOrig = p.GetTime();

      // beam nucleon number
      const int kABeam = GetNucleusA(corsikaBeamId);
      // TODO: verify this number !!!
      if(kABeam>56)
	throw std::runtime_error("Beam nucleus too large for SIBYLL!");
      
      // kinematics
      // define projectile NUCLEON
      HEPEnergyType const eProjectileLab = p.GetEnergy() / kABeam;
      auto const pProjectileLab = p.GetMomentum() / kABeam;
      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      cout << "NuclearInteraction: ebeam lab: " << eProjectileLab / 1_GeV << endl
	   << "NuclearInteraction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
	   << endl;

      // define target
      // always a nucleon
      // for Sibyll is always a single nucleon
      auto constexpr nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
					   corsika::particles::Neutron::GetMass());
      // target is always at rest
      const auto eTargetLab = 0_GeV + nucleon_mass;
      const auto pTargetLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);
      
      cout << "NuclearInteraction: etarget lab: " << eTargetLab / 1_GeV << endl
	   << "NuclearInteraction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV
	   << endl;

      // center-of-mass energy in nucleon-nucleon frame
      auto const Ptot4 = PtargLab + PprojLab;
      HEPEnergyType Ecm = Ptot4.GetNorm();
      cout << "NuclearInteraction: nuc-nuc cm energy: " << Ecm / 1_GeV << endl;

      const auto beamId = corsika::particles::Proton::GetCode();
      // sample target nucleon number
      const auto currentNode = fEnvironment.GetUniverse()->GetContainingNode(pOrig);
      const auto& mediumComposition =
	currentNode->GetModelProperties().GetNuclearComposition();
      // get cross sections for target materials
      /*
	Here we read the cross section from the interaction model again,
	should be passed from GetInteractionLength if possible
      */
      auto const& compVec = mediumComposition.GetComponents();
      std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
	auto const targetId = compVec[i];
	const auto [sigProd, nNuc] = GetCrossSection(beamId,targetId,Ecm);
	cross_section_of_components[i] = sigProd;
      }

      const auto targetCode = currentNode->GetModelProperties().SampleTarget(
									     cross_section_of_components, fRNG);
      cout << "Interaction: target selected: " << targetCode << endl;
      /*
	FOR NOW: allow nuclei with A<18 or protons only.
	when medium composition becomes more complex, approximations will have to be
	allowed air in atmosphere also contains some Argon.
      */
      int kATarget = -1;
      if (IsNucleus(targetCode)) kATarget = GetNucleusA(targetCode);
      if (targetCode == corsika::particles::Proton::GetCode()) kATarget = 1;
      cout << "NuclearInteraction: sibyll target code: " << kATarget << endl;
      if (kATarget > 18 || kATarget < 1)
	throw std::runtime_error(
				 "Sibyll target outside range. Only nuclei with A<18 or protons are "
				 "allowed.");
      // end of target sampling

      // superposition 
      cout << "NuclearInteraction: sampling nuc. multiple interaction structure.. "<< endl;
      // get nucleon-nucleon cross section
      // (needed to determine number of scatterings)
      double sigProd, sigEla, dum1, dum2, dum3, dum4;
      double dumdif[3];
      const double sqsnuc = Ecm / 1_GeV;
      const int iBeam = 1;
      sib_sigma_hp_(iBeam, sqsnuc, dum1, sigEla, sigProd, dumdif, dum3, dum4);	
      
      // sample number of interactions (output in common cnucms)
      // nuclear multiple scattering according to glauber (r.i.p.)
      int_nuc_( kATarget, kABeam, sigProd, sigEla);

      cout << "number of wounded nucleons in target   : " << cnucms_.na << endl
	   << "number of wounded nucleons in project. : " << cnucms_.nb << endl
	   << "number of inel. nuc.-nuc. interactions : " << cnucms_.ni << endl
	   << "number of elastic nucleons in target   : " << cnucms_.nael << endl
	   << "number of elastic nucleons in project. : " << cnucms_.nbel << endl;
      
      throw std::runtime_error(" end now");      

      // calculate fragmentation
      // call FRAGM (IAT,IAP, NW,B, NF, IAF)
      // fragm_(kATarget, kABeam, NBT, B, NF, IAF)

      /*
C.  INPUT: IAP = mass of incident nucleus
C.         IAT = mass of target   nucleus
C.         NW = number of wounded nucleons in the beam nucleus
C.         B  = impact parameter in the interaction
C.     
C.  OUTPUT : NF = number of fragments  of the spectator nucleus
C.           IAF(1:NF) = mass number of each fragment
C.           PF(3,60) in common block /FRAGMENTS/ contains
C.           the three momentum components (MeV/c) of each
C.           fragment in the projectile frame
C..............................................................
       */
      
      // put spectators on intermediate stack
      //Stack nucs;
      
      // add elastic nucleons to stack

      // add inelastic interactions
      

      // move particles to corsika stack
      // boost

      
      // add proton instead
      auto pnew = s.NewParticle();
      pnew.SetPID( corsika::particles::Proton::GetCode() );
      pnew.SetMomentum( p.GetMomentum() );
      pnew.SetEnergy( p.GetEnergy() );	

      // delete current particle
      p.Delete();

      return process::EProcessReturn::eOk;
    }

  private:
    corsika::environment::Environment const& fEnvironment;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  };

} // namespace corsika::process::sibyll

#endif
