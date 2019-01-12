
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_sibyll_interaction_h_
#define _corsika_process_sibyll_interaction_h_

#include <corsika/process/InteractionProcess.h>

#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>
#include <corsika/process/sibyll/sibyll2.3c.h>
#include <corsika/utl/COMBoost.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>

namespace corsika::process::sibyll {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    int fCount = 0;
    int fNucCount = 0;
  public:
    Interaction(corsika::environment::Environment const& env) : fEnvironment(env) { }
    ~Interaction() {
      std::cout << "Sibyll::Interaction n=" << fCount << " Nnuc=" << fNucCount
                << std::endl;
    }

    void Init() {

      using corsika::random::RNGManager;
      using std::cout;
      using std::endl;

      corsika::random::RNGManager& rmng = corsika::random::RNGManager::GetInstance();
      rmng.RegisterRandomStream("s_rndm");
     
      // initialize Sibyll
      sibyll_ini_();
    }

    std::tuple<corsika::units::si::CrossSectionType, int> GetCrossSection(const corsika::particles::Code &BeamId, const corsika::particles::Code & TargetId, const corsika::units::si::HEPEnergyType& CoMenergy)
    {
      using namespace corsika::units::si;
      double sigProd, dummy, dum1, dum2, dum3, dum4;
      double dumdif[3];
      const int iBeam = process::sibyll::GetSibyllXSCode(BeamId);
      const double dEcm = CoMenergy / 1_GeV;
      if(corsika::particles::IsNucleus( TargetId )){
	const int iTarget = corsika::particles::GetNucleusA( TargetId );
	if(iTarget>18||iTarget==0)
	  throw std::runtime_error("Sibyll target outside range. Only nuclei with A<18 are allowed.");
	sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy);
	return std::make_tuple( sigProd * 1_mbarn, iTarget );
      }
      else
	{
	  if (TargetId == corsika::particles::Proton::GetCode()){
	    sib_sigma_hp_(iBeam, dEcm, dum1, dum2, sigProd, dumdif, dum3, dum4);
	    return std::make_tuple( sigProd * 1_mbarn, 1 );
	  }
	  else
	    // no interaction in sibyll possible, return infinite cross section? or throw?
	    sigProd = std::numeric_limits<double>::infinity();
	  return std::make_tuple( sigProd * 1_mbarn, 0 );
	}
    }
    
    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle& p, Track&) {

      using namespace corsika::units;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      // coordinate system, get global frame of reference
      CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      const particles::Code corsikaBeamId = p.GetPID();

      // beam particles for sibyll : 1, 2, 3 for p, pi, k
      // read from cross section code table
      const bool kInteraction = process::sibyll::CanInteract(corsikaBeamId);
      
      const HEPMassType nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                                corsika::particles::Neutron::GetMass());

      // FOR NOW: assume target is at rest
      MomentumVector pTarget(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});

      // total momentum and energy
      HEPEnergyType Elab = p.GetEnergy() + nucleon_mass;
      MomentumVector Ptot(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      Ptot += p.GetMomentum();
      Ptot += pTarget;
      // calculate cm. energy
      const HEPEnergyType ECoM = sqrt(Elab * Elab - Ptot.squaredNorm());

      std::cout << "Interaction: LambdaInt: \n"
                << " input energy: " << p.GetEnergy() / 1_GeV << endl
                << " beam can interact:" << kInteraction << endl
                << " beam pid:" << p.GetPID() << endl;

      if (kInteraction && Elab >= 8.5_GeV && ECoM >= 10_GeV) {

	// get target from environment
	/*
	  the target should be defined by the Environment,
	  ideally as full particle object so that the four momenta
	  and the boosts can be defined..
	*/
	const auto currentNode = fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
	const auto mediumComposition = currentNode->GetModelProperties().GetNuclearComposition();
	// determine average interaction length
	// weighted sum
	int i =-1;
	double avgTargetMassNumber = 0.;
	si::CrossSectionType weightedProdCrossSection = 0_mbarn;
	// get weights of components from environment/medium
	const auto w = mediumComposition.GetFractions();
	// loop over components in medium
	for (auto targetId: mediumComposition.GetComponents() ){
	  i++;
	  cout << "Interaction: get interaction length for target: " << targetId << endl;

	  auto const [ productionCrossSection, numberOfNucleons ] = GetCrossSection( corsikaBeamId, targetId, ECoM);
	  
	  std::cout << "Interaction: "
		    << " IntLength: sibyll return (mb): " << productionCrossSection / 1_mbarn << std::endl;
	  weightedProdCrossSection += w[i] * productionCrossSection;
	  avgTargetMassNumber += w[i] * numberOfNucleons;
	}
	cout << "Interaction: "
	     << "IntLength: weighted CrossSection (mb): " << weightedProdCrossSection / 1_mbarn
	     << endl;
	
	// calculate interaction length in medium
#warning check interaction length units	
	GrammageType int_length = avgTargetMassNumber * corsika::units::constants::u / weightedProdCrossSection;
	std::cout << "Interaction: "
		  << "interaction length (g/cm2): " << int_length / ( 0.001_kg ) * 1_cm * 1_cm << std::endl;

        return int_length;
      }

      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
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
      cout << "ProcessSibyll: "
           << "DoInteraction: " << corsikaBeamId << " interaction? "
           << process::sibyll::CanInteract( corsikaBeamId ) << endl;
      if (process::sibyll::CanInteract(corsikaBeamId) ) {
        const CoordinateSystem& rootCS =
            RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

        Point pOrig = p.GetPosition();
        TimeType tOrig = p.GetTime();
        // sibyll CS has z along particle momentum
        // FOR NOW: hard coded z-axis for corsika frame
        QuantityVector<length_d> const zAxis{0_m, 0_m, 1_m};
        QuantityVector<length_d> const yAxis{0_m, 1_m, 0_m};
        auto rotation_angles = [](MomentumVector const& pin) {
          const auto p = pin.GetComponents();
          const auto th = acos(p[2] / p.norm());
          const auto ph = atan2(
              p[1] / 1_GeV, p[0] / 1_GeV); // acos( p[0] / sqrt(p[0]*p[0]+p[1]*p[1] ) );
          return std::make_tuple(th, ph);
        };
        // auto pt = []( MomentumVector &p ){
        // 	    return sqrt(p.GetComponents()[0] * p.GetComponents()[0] +
        // p.GetComponents()[1] * p.GetComponents()[1]);
        // 	  };
        // double theta = acos( p.GetMomentum().GetComponents()[2] /
        // p.GetMomentum().norm());
        auto const [theta, phi] = rotation_angles(p.GetMomentum());
        cout << "ProcessSibyll: zenith angle between sibyllCS and rootCS: "
             << theta / M_PI * 180. << endl;
        cout << "ProcessSibyll: azimuth angle between sibyllCS and rootCS: "
             << phi / M_PI * 180. << endl;
        // double phi = asin( p.GetMomentum().GetComponents()[0]/pt(p.GetMomentum() ) );
        const CoordinateSystem tempCS = rootCS.rotate(zAxis, phi);
        const CoordinateSystem sibyllCS = tempCS.rotate(yAxis, theta);
	
	

        // FOR NOW: target is always at rest
        auto constexpr nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                         corsika::particles::Neutron::GetMass());
        const auto Etarget = 0_GeV + nucleon_mass;
        const auto pTarget = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
        cout << "target momentum (GeV/c): " << pTarget.GetComponents() / 1_GeV << endl;
        cout << "beam momentum (GeV/c): " << p.GetMomentum().GetComponents() / 1_GeV
             << endl;
        cout << "beam momentum in sibyll frame (GeV/c): "
             << p.GetMomentum().GetComponents(sibyllCS) / 1_GeV << endl;

        cout << "position of interaction: " << pOrig.GetCoordinates() << endl;
        cout << "time: " << tOrig << endl;

        // get energy of particle from stack
        /*
          stack is in GeV in lab. frame
          convert to GeV in cm. frame
          (assuming proton at rest as target AND
          assuming no pT, i.e. shower frame-z is aligned with hadron-int-frame-z)
        */
        // total energy: E_beam + E_target
        // in lab. frame: E_beam + m_target*c**2
        HEPEnergyType E = p.GetEnergy();
        HEPEnergyType Etot = E + Etarget;
        // total momentum
        MomentumVector Ptot = p.GetMomentum();
        // invariant mass, i.e. cm. energy
        HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.squaredNorm());

	
	// sample target mass number
	const auto currentNode = fEnvironment.GetUniverse()->GetContainingNode(pOrig);
	const auto mediumComposition = currentNode->GetModelProperties().GetNuclearComposition();
	// get cross sections for target materials
	/*
	  Here we read the cross section from the interaction model again, 
	  should be passed from GetInteractionLength if possible
	 */
#warning reading interaction cross section again, should not be necessary
	std::vector<si::CrossSectionType> cross_section_of_components;
	for(auto targetId: mediumComposition.GetComponents() ){
	  const auto [sigProd, nNuc] = GetCrossSection( corsikaBeamId, targetId, Ecm);	  
	  cross_section_of_components.push_back( sigProd );
	}	

	const auto targetCode = currentNode->GetModelProperties().GetTarget( cross_section_of_components);
	cout << "Interaction: target selected: " << targetCode << endl;
	/*
	  FOR NOW: allow nuclei with A<18 or protons only. 
	  when medium composition becomes more complex, approximations will have to be allowed
	  air in atmosphere also contains some Argon.
	*/
	int targetSibCode = -1;
	if( IsNucleus(targetCode))
	  targetSibCode = GetNucleusA( targetCode );
	if( targetCode == corsika::particles::Proton::GetCode())
	  targetSibCode = 1;
	cout << "Interaction: sibyll code: " << targetSibCode << endl;
	if(targetSibCode>18||targetSibCode<1)
	  throw std::runtime_error("Sibyll target outside range. Only nuclei with A<18 or protons are allowed.");
	
        /*
         get transformation between Stack-frame and SibStack-frame
         for EAS Stack-frame is lab. frame, could be different for CRMC-mode
         the transformation should be derived from the input momenta
       */
        const double gamma = Etot / Ecm;
        const auto gambet = Ptot / Ecm;

        std::cout << "Interaction: "
                  << " DoDiscrete: gamma:" << gamma << endl;
        std::cout << "Interaction: "
                  << " DoDiscrete: gambet:" << gambet.GetComponents() << endl;

	auto const pProjectileLab = p.GetMomentum();
	//{rootCS, {0_GeV / c, 1_PeV / c, 0_GeV / c}};
	HEPEnergyType const eProjectileLab = p.GetEnergy();
	  //energy(projectileMass, pProjectileLab);

	// define target kinematics in lab frame
	HEPMassType const targetMass = nucleon_mass;
	// define boost to com frame
	COMBoost const boost(eProjectileLab, pProjectileLab, targetMass);

	cout << "Interaction: new boost: ebeam lab: " << eProjectileLab / 1_GeV << endl
	     << "Interaction: new boost: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV << endl;

	// boost projecticle
	auto const [eProjectileCoM, pProjectileCoM] =
	  boost.toCoM(eProjectileLab, pProjectileLab);

	cout << "Interaction: new boost: ebeam com: " << eProjectileCoM / 1_GeV << endl
	     << "Interaction: new boost: pbeam com: " << pProjectileCoM / 1_GeV << endl;
	
        const int kBeam = process::sibyll::ConvertToSibyllRaw(corsikaBeamId);

        std::cout << "Interaction: "
                  << " DoInteraction: E(GeV):" << E / 1_GeV
                  << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;
        if (E < 8.5_GeV || Ecm < 10_GeV) {
          std::cout << "Interaction: "
                    << " DoInteraction: should have dropped particle.." << std::endl;
          // p.Delete(); delete later... different process
        } else {
          fCount++;
          // Sibyll does not know about units..
          const double sqs = Ecm / 1_GeV;
          // running sibyll, filling stack
          sibyll_(kBeam, targetSibCode, sqs);
          // running decays
          // setTrackedParticlesStable();
          decsib_();
          // print final state
          int print_unit = 6;
          sib_list_(print_unit);
          fNucCount += get_nwounded() - 1;

          // delete current particle
          p.Delete();

          // add particles from sibyll to stack
          // link to sibyll stack
          // here we need to pass projectile momentum and energy to define the local
          // sibyll frame and the boosts to the lab. frame
          SibStack ss;

          // momentum array in Sibyll
          MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
          HEPEnergyType E_final = 0_GeV, Ecm_final = 0_GeV;
          for (auto& psib : ss) {

            // skip particles that have decayed in Sibyll
            if (psib.HasDecayed()) continue;

            // transform energy to lab. frame, primitve
            // compute beta_vec * p_vec
            // arbitrary Lorentz transformation based on sibyll routines
            const auto gammaBetaComponents = gambet.GetComponents();
            // FOR NOW: fill vector in sibCS and then rotate into rootCS
            // can be done in SibStack by passing sibCS
            // get momentum vector in sibyllCS
            const auto pSibyllComponentsSibCS = psib.GetMomentum().GetComponents();
            // temporary vector in sibyllCS
            auto SibVector = MomentumVector(sibyllCS, pSibyllComponentsSibCS);
            // rotatate to rootCS
            const auto pSibyllComponents = SibVector.GetComponents(rootCS);
            // boost to lab. frame
            HEPEnergyType en_lab = 0. * 1_GeV;
            HEPMomentumType p_lab_components[3];
            en_lab = psib.GetEnergy() * gamma;
            HEPEnergyType pnorm = 0. * 1_GeV;
            for (int j = 0; j < 3; ++j)
              pnorm += (pSibyllComponents[j] * gammaBetaComponents[j]) / (gamma + 1.);
            pnorm += psib.GetEnergy();

            for (int j = 0; j < 3; ++j) {
              p_lab_components[j] =
                  pSibyllComponents[j] - (-1) * pnorm * gammaBetaComponents[j];
              en_lab -= (-1) * pSibyllComponents[j] * gammaBetaComponents[j];
            }

            // add to corsika stack
            auto pnew = s.NewParticle();
            pnew.SetEnergy(en_lab);
            pnew.SetPID(process::sibyll::ConvertFromSibyll(psib.GetPID()));
            corsika::geometry::QuantityVector<hepmomentum_d> p_lab_c{
                p_lab_components[0], p_lab_components[1], p_lab_components[2]};
            pnew.SetMomentum(MomentumVector(rootCS, p_lab_c));
            pnew.SetPosition(pOrig);
            pnew.SetTime(tOrig);
            Plab_final += pnew.GetMomentum();
            E_final += en_lab;
            Ecm_final += psib.GetEnergy();
          }
          std::cout << "conservation (all GeV): E_final=" << E_final / 1_GeV
                    << ", Ecm_final=" << Ecm_final / 1_GeV
                    << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents()
                    << std::endl;
        }
      }
      return process::EProcessReturn::eOk;
    }
    
  private:
    corsika::environment::Environment const& fEnvironment;

  };

} // namespace corsika::process::sibyll

#endif
