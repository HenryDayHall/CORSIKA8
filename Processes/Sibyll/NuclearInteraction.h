
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
      
      if(!corsika::particles::IsNucleus(BeamId)){
	sigProd = std::numeric_limits<double>::infinity();
	return std::make_tuple(sigProd * 1_mbarn, 1);
      }

      // TODO: use nuclib to calc. nuclear cross sections
      // FOR NOW: use proton cross section for nuclei
      auto const BeamIdToUse = corsika::particles::Proton::GetCode();
      std::cout << "WARNING: replacing beam nucleus with proton!" << std::endl;
      
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

      if(!corsika::particles::IsNucleus(corsikaBeamId))
	// no nuclear interaction
	return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);

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
      const auto kNucleus = IsNucleus(corsikaBeamId);
      if(!kNucleus)
	return process::EProcessReturn::eOk;
      

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
