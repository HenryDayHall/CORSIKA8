
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
#include <corsika/process/sibyll/nuclib.h>
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
      //sibyll_ini_();
      
      // initialize nuclib
      nuc_nuc_ini_();
    }

    std::tuple<corsika::units::si::CrossSectionType, int> GetCrossSection(
        const corsika::particles::Code BeamId, const corsika::particles::Code TargetId,
        const corsika::units::si::HEPEnergyType CoMenergy) {
      using namespace corsika::units::si;
      double sigProd, dummy, dum1, dum2, dum3, dum4;
      double dumdif[3];

      if(!corsika::particles::IsNucleus(BeamId)){
	// return infinite cross section, no interaction
	return std::make_tuple( std::numeric_limits<double>::infinity() * 1_mbarn, 0);
      }
      
      // TODO: use nuclib to calc. nuclear cross sections
      // FOR NOW: use proton cross section for nuclei
      corsika::particles::Code BeamIdToUse;
      BeamIdToUse = corsika::particles::Proton::GetCode();
      std::cout << "WARNING: replacing beam nucleus with proton!" << std::endl;
      
      const int iBeam = process::sibyll::GetSibyllXSCode(BeamIdToUse);
      const double dEcm = CoMenergy / 1_GeV;
      // check target, hadron or nucleus?
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

      // this routine superimposes different nucleon-nucleon interactions
      // in a nucleus-nucleus interaction, based the SIBYLL routine SIBNUC 

      using namespace corsika::units;
      using namespace corsika::utl;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      const auto corsikaProjId = p.GetPID();
      cout << "NuclearInteraction: DoInteraction: called with:" << corsikaProjId << endl
	   << "NuclearInteraction: projectile mass: " << corsika::particles::GetMass(corsikaProjId) / 1_GeV
	   << endl;
      
      if(!IsNucleus(corsikaProjId)){
	// this should not happen
	throw std::runtime_error("Non nuclear projectile in NUCLIB!");
      }

      fCount++;
      
      const CoordinateSystem& rootCS =
	RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
      
      // position and time of interaction, not used in Sibyll
      Point pOrig = p.GetPosition();
      TimeType tOrig = p.GetTime();

      cout << "Interaction: position of interaction: " << pOrig.GetCoordinates()
	   << endl;
      cout << "Interaction: time: " << tOrig << endl;      
      
      // projectile nucleon number
      const int kAProj = GetNucleusA(corsikaProjId);
      if(kAProj>56)
	throw std::runtime_error("Projectile nucleus too large for NUCLIB!");
      
      // kinematics
      // define projectile nucleus
      HEPEnergyType const eProjectileLab = p.GetEnergy();
      auto const pProjectileLab = p.GetMomentum();
      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      cout << "NuclearInteraction: eProj lab: " << eProjectileLab / 1_GeV << endl
	   << "NuclearInteraction: pProj lab: " << pProjectileLab.GetComponents() / 1_GeV
	   << endl;

      // define projectile nucleon
      HEPEnergyType const eProjectileNucLab = p.GetEnergy() / kAProj;
      auto const pProjectileNucLab = p.GetMomentum() / kAProj;
      const FourVector PprojNucLab(eProjectileNucLab, pProjectileNucLab);

      cout << "NuclearInteraction: eProjNucleon lab: " << eProjectileNucLab / 1_GeV << endl
	   << "NuclearInteraction: pProjNucleon lab: " << pProjectileNucLab.GetComponents() / 1_GeV
	   << endl;


      // define target
      // always a nucleon
      // for Sibyll is always a single nucleon
      auto constexpr nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
					   corsika::particles::Neutron::GetMass());
      // target is always at rest
      const auto eTargetNucLab = 0_GeV + nucleon_mass;
      const auto pTargetNucLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargNucLab(eTargetNucLab, pTargetNucLab);
      
      cout << "NuclearInteraction: etarget lab: " << eTargetNucLab / 1_GeV << endl
	   << "NuclearInteraction: ptarget lab: " << pTargetNucLab.GetComponents() / 1_GeV
	   << endl;

      // center-of-mass energy in nucleon-nucleon frame
      auto const PtotNN4 = PtargNucLab + PprojNucLab;
      HEPEnergyType EcmNN = PtotNN4.GetNorm();
      cout << "NuclearInteraction: nuc-nuc cm energy: " << EcmNN / 1_GeV << endl;

      // define boost to NUCLEON-NUCLEON frame
      COMBoost const boost(PprojNucLab, nucleon_mass);
      // boost projecticle
      auto const PprojNucCoM = boost.toCoM(PprojNucLab);

      // boost target
      auto const PtargNucCoM = boost.toCoM(PtargNucLab);

      cout << "Interaction: ebeam CoM: " << PprojNucCoM.GetTimeLikeComponent() / 1_GeV
	   << endl
	   << "Interaction: pbeam CoM: "
	   << PprojNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;
      cout << "Interaction: etarget CoM: " << PtargNucCoM.GetTimeLikeComponent() / 1_GeV
	   << endl
	   << "Interaction: ptarget CoM: "
	   << PtargNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;

      // sample target nucleon number
      //
      // proton stand-in for nucleon
      const auto beamId = corsika::particles::Proton::GetCode();
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
	const auto [sigProd, nNuc] = GetCrossSection(beamId,targetId,EcmNN);
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
      //
      // TODO: this is an explicit dependence on sibyll
      //       should be changed to a call to GetCrossSection() of the had. int. implementation
      //       this also means GetCrossSection has to return the elastic cross section as well
      double sigProd, sigEla, dum1, dum2, dum3;
      double dumdif[3];
      const double sqsnuc = EcmNN / 1_GeV;
      const int iBeam = 1;
      // read hadron-proton cross section table (input: hadron id, CoMenergy, output: cross sections)
      sib_sigma_hp_(iBeam, sqsnuc, dum1, sigEla, sigProd, dumdif, dum2, dum3);	
      
      // sample number of interactions (only input variables, output in common cnucms)
      // nuclear multiple scattering according to glauber (r.i.p.)      
      int_nuc_( kATarget, kAProj, sigProd, sigEla);

      cout << "number of nucleons in target           : " << kATarget << endl
	   << "number of wounded nucleons in target   : " << cnucms_.na << endl
	   << "number of nucleons in projectile       : " << kAProj << endl
	   << "number of wounded nucleons in project. : " << cnucms_.nb << endl
	   << "number of inel. nuc.-nuc. interactions : " << cnucms_.ni << endl
	   << "number of elastic nucleons in target   : " << cnucms_.nael << endl
	   << "number of elastic nucleons in project. : " << cnucms_.nbel << endl
	   << "impact parameter: " << cnucms_.b << endl;      
      
      // calculate fragmentation
      cout << "calculating nuclear fragments.." << endl;
      // number of interactions
      // include elastic
      const int nElasticNucleons = cnucms_.nbel;
      const int nInelNucleons = cnucms_.nb;
      const int nIntProj = nInelNucleons + nElasticNucleons;
      const double impactPar = cnucms_.b; // only needed to avoid passing common var.
      int nFragments;
      // number of fragments is limited to 60      
      int AFragments[60];
      // call fragmentation routine
      // input: target A, projectile A, number of int. nucleons in projectile, impact parameter (fm)
      // output: nFragments, AFragments
      // in addition the momenta ar stored in pf in common fragments, neglected
      fragm_(kATarget, kAProj, nIntProj, impactPar, nFragments, AFragments);

      // this should not occur but well :)
      if(nFragments>60)
	throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");

      cout << "number of fragments: " << nFragments << endl;
      for(int j=0; j<nFragments; ++j)	
	cout << "fragment: " << j << " A=" << AFragments[j]
	     << " px=" << fragments_.ppp[j][0]
	     << " py=" << fragments_.ppp[j][1]
	     << " pz=" << fragments_.ppp[j][2] 
	     << endl;
      
      // bookeeping accross nucleon-nucleon interactions
      MomentumVector Plab_all(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_all = 0_GeV;

      
      auto Nucleus = []( int iA ){
		       //		       int znew = iA / 2.15 + 0.7;
		       corsika::particles::Code pCode;		       
		       switch( iA ){
		       case 2:
			 pCode = corsika::particles::Deuterium::GetCode();
			 break;
			 
		       case 3:
			 pCode = corsika::particles::Tritium::GetCode();
			 break;

		       case 4:
			 pCode = corsika::particles::Helium::GetCode();
			 break;
			 
		       case 12:
			 pCode = corsika::particles::Carbon::GetCode();
			 break;

		       case 14:
			 pCode = corsika::particles::Nitrogen::GetCode();
			 break;

		       case 16:
			 pCode = corsika::particles::Oxygen::GetCode();
			 break;

		       case 33:
			 pCode = corsika::particles::Sulphur::GetCode();
			 break;
			 
		       case 40:
			 pCode = corsika::particles::Argon::GetCode();
			 break;
			 
		       default:
			 pCode = corsika::particles::Proton::GetCode();
		       }
		       return pCode;
		     };
      
      // put nuclear fragments on corsika stack
      for(int j=0; j<nFragments; ++j){
	auto pnew = s.NewParticle();
	// here we need the nucleonNumber to corsika Id conversion
	// A =  AFragments[j]
	// pnew.SetPID( corsika::particles::GetCode( corsika::particles::Nucleus(A,Z) ) );
	auto pCode = Nucleus( AFragments[j] );
	pnew.SetPID(pCode);
	
	// CORSIKA 7 way
	// spectators inherit momentum from original projectile
	const double mass_ratio = corsika::particles::GetMass( pCode ) / corsika::particles::GetMass( corsikaProjId );
	auto const Plab = PprojLab * mass_ratio;
	  
	pnew.SetEnergy(Plab.GetTimeLikeComponent());
	pnew.SetMomentum(Plab.GetSpaceLikeComponents());
	pnew.SetPosition(pOrig);
	pnew.SetTime(tOrig);

	Plab_all += Plab.GetSpaceLikeComponents();
	Elab_all += Plab.GetTimeLikeComponent();
      }
      
      // add elastic nucleons to corsika stack
      for(int j=0; j<nElasticNucleons; ++j){
	auto pnew = s.NewParticle();
	// TODO: sample proton or neutron
	auto pCode = corsika::particles::Proton::GetCode();
	pnew.SetPID( pCode );

	// CORSIKA 7 way
	// elastic nucleons inherit momentum from original projectile
	// neglecting momentum transfer in interaction
	const double mass_ratio = corsika::particles::GetMass( pCode ) / corsika::particles::GetMass( corsikaProjId );
	auto const Plab = PprojLab * mass_ratio;

	pnew.SetEnergy(Plab.GetTimeLikeComponent());
	pnew.SetMomentum(Plab.GetSpaceLikeComponents());
	pnew.SetPosition(pOrig);
	pnew.SetTime(tOrig);

	Plab_all += Plab.GetSpaceLikeComponents();
	Elab_all += Plab.GetTimeLikeComponent();
      }

      // add inelastic interactions
      for(int j=0; j<nInelNucleons; ++j){

	// create nucleon-nucleus inelastic interaction
	// TODO: switch to DoInteraction() of had. interaction implementation
	// for now use SIBYLL directly
	const int kBeamCode = process::sibyll::ConvertToSibyllRaw(corsika::particles::Proton::GetCode());
	cout << "creating interaction no. "<< j << endl;
	sibyll_( kBeamCode, kATarget, sqsnuc );
	decsib_();
	// print final state
	int print_unit = 6;
	sib_list_(print_unit);

	// add particles from sibyll to stack
	// link to sibyll stack
	SibStack ss;

	MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
	HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
	for (auto& psib : ss) {

	  // skip particles that have decayed in Sibyll
	  if (psib.HasDecayed()) continue;

	  // // transform energy to lab. frame
	  auto const pCoM = psib.GetMomentum();
	  HEPEnergyType const eCoM = psib.GetEnergy();
	  auto const Plab = boost.fromCoM(FourVector(eCoM, pCoM));
	  
	  // add to corsika stack
	  auto pnew = s.NewParticle();
	  pnew.SetPID(process::sibyll::ConvertFromSibyll(psib.GetPID()));
	  pnew.SetEnergy(Plab.GetTimeLikeComponent());
	  pnew.SetMomentum(Plab.GetSpaceLikeComponents());
	  pnew.SetPosition(pOrig);
	  pnew.SetTime(tOrig);

	  Plab_final += pnew.GetMomentum();
	  Elab_final += pnew.GetEnergy();
	  Ecm_final += psib.GetEnergy();

	  Plab_all += Plab.GetSpaceLikeComponents();
	  Elab_all += Plab.GetTimeLikeComponent();
	}
	cout << "conservation (all GeV): Ecm_final=" << Ecm_final / 1_GeV
		  << endl
		  << "Elab_final=" << Elab_final / 1_GeV
		  << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents()
		  << endl;	
      }
      cout << "accross all nucleon interactions: Etot lab: " << Elab_all / 1_GeV << endl
	   << "accross all nucleon interactions: Ptot lab: " << (Plab_all / 1_GeV).GetComponents()
	   << endl;
      
      //throw std::runtime_error(" stop here");

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
