
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

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>
#include <corsika/process/sibyll/sibyll2.3c.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

namespace corsika::process::sibyll {

  class Interaction : public corsika::process::InteractionProcess<Interaction> {

    int fCount = 0;
    int fNucCount = 0;

  public:
    Interaction(corsika::environment::Environment const& env)
        : fEnvironment(env) {}
    ~Interaction() {
      std::cout << "Sibyll::Interaction n=" << fCount << " Nnuc=" << fNucCount
                << std::endl;
    }

    void Init() {

      using corsika::random::RNGManager;
      using std::cout;
      using std::endl;

      // initialize Sibyll
      sibyll_ini_();
    }

    std::tuple<corsika::units::si::CrossSectionType, int> GetCrossSection(
        const corsika::particles::Code BeamId, const corsika::particles::Code TargetId,
        const corsika::units::si::HEPEnergyType CoMenergy) {
      using namespace corsika::units::si;
      double sigProd, dummy, dum1, dum2, dum3, dum4;
      double dumdif[3];
      const int iBeam = process::sibyll::GetSibyllXSCode(BeamId);
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

      // beam particles for sibyll : 1, 2, 3 for p, pi, k
      // read from cross section code table
      const bool kInteraction = process::sibyll::CanInteract(corsikaBeamId);

      const HEPMassType nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                              corsika::particles::Neutron::GetMass());

      // FOR NOW: assume target is at rest
      MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

      // total momentum and energy
      HEPEnergyType Elab = p.GetEnergy() + nucleon_mass;
      MomentumVector pTotLab(rootCS, {0_GeV, 0_GeV, 0_GeV});
      pTotLab += p.GetMomentum();
      pTotLab += pTarget;
      auto const pTotLabNorm = pTotLab.norm();
      // calculate cm. energy
      const HEPEnergyType ECoM = sqrt(
          (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

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
          cout << "Interaction: get interaction length for target: " << targetId << endl;

          auto const [productionCrossSection, numberOfNucleons] =
              GetCrossSection(corsikaBeamId, targetId, ECoM);

          std::cout << "Interaction: "
                    << " IntLength: sibyll return (mb): "
                    << productionCrossSection / 1_mbarn << std::endl;
          weightedProdCrossSection += w[i] * productionCrossSection;
          avgTargetMassNumber += w[i] * numberOfNucleons;
        }
        cout << "Interaction: "
             << "IntLength: weighted CrossSection (mb): "
             << weightedProdCrossSection / 1_mbarn << endl;

        // calculate interaction length in medium
#warning check interaction length units
        GrammageType const int_length =
            avgTargetMassNumber * corsika::units::constants::u / weightedProdCrossSection;
        std::cout << "Interaction: "
                  << "interaction length (g/cm2): "
                  << int_length / (0.001_kg) * 1_cm * 1_cm << std::endl;

        return int_length;
      }

      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }

    /**
       In this function SIBYLL is called to produce one event. The
       event is copied (and boosted) into the shower lab frame.
     */

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
           << process::sibyll::CanInteract(corsikaBeamId) << endl;
      
      if(corsika::particles::IsNucleus(corsikaBeamId)){
	// nuclei handled by different process, this should not happen
	throw std::runtime_error("Nuclear projectile are not handled by SIBYLL!");
      }
      
      if (process::sibyll::CanInteract(corsikaBeamId)) {
        const CoordinateSystem& rootCS =
            RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

        // position and time of interaction, not used in Sibyll
        Point pOrig = p.GetPosition();
        TimeType tOrig = p.GetTime();

        // define target
        // for Sibyll is always a single nucleon
        auto constexpr nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                             corsika::particles::Neutron::GetMass());
        // FOR NOW: target is always at rest
        const auto eTargetLab = 0_GeV + nucleon_mass;
        const auto pTargetLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
        const FourVector PtargLab(eTargetLab, pTargetLab);

        // define projectile
        HEPEnergyType const eProjectileLab = p.GetEnergy();
        auto const pProjectileLab = p.GetMomentum();

        cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << endl
             << "Interaction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
             << endl;
        cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << endl
             << "Interaction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV
             << endl;

        const FourVector PprojLab(eProjectileLab, pProjectileLab);

        // define target kinematics in lab frame
        // define boost to and from CoM frame
        // CoM frame definition in Sibyll projectile: +z
        COMBoost const boost(PprojLab, nucleon_mass);

        // just for show:
        // boost projecticle
        auto const PprojCoM = boost.toCoM(PprojLab);

        // boost target
        auto const PtargCoM = boost.toCoM(PtargLab);

        cout << "Interaction: ebeam CoM: " << PprojCoM.GetTimeLikeComponent() / 1_GeV
             << endl
             << "Interaction: pbeam CoM: "
             << PprojCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;
        cout << "Interaction: etarget CoM: " << PtargCoM.GetTimeLikeComponent() / 1_GeV
             << endl
             << "Interaction: ptarget CoM: "
             << PtargCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;

        cout << "Interaction: position of interaction: " << pOrig.GetCoordinates()
             << endl;
        cout << "Interaction: time: " << tOrig << endl;

        HEPEnergyType Etot = eProjectileLab + eTargetLab;
        MomentumVector Ptot = p.GetMomentum();
        // invariant mass, i.e. cm. energy
        HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.squaredNorm());

        // sample target mass number
        const auto currentNode = fEnvironment.GetUniverse()->GetContainingNode(pOrig);
        const auto& mediumComposition =
            currentNode->GetModelProperties().GetNuclearComposition();
        // get cross sections for target materials
        /*
          Here we read the cross section from the interaction model again,
          should be passed from GetInteractionLength if possible
         */
#warning reading interaction cross section again, should not be necessary
        auto const& compVec = mediumComposition.GetComponents();
        std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

        for (size_t i = 0; i < compVec.size(); ++i) {
          auto const targetId = compVec[i];
          const auto [sigProd, nNuc] = GetCrossSection(corsikaBeamId, targetId, Ecm);
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
        int targetSibCode = -1;
        if (IsNucleus(targetCode)) targetSibCode = GetNucleusA(targetCode);
        if (targetCode == corsika::particles::Proton::GetCode()) targetSibCode = 1;
        cout << "Interaction: sibyll code: " << targetSibCode << endl;
        if (targetSibCode > 18 || targetSibCode < 1)
          throw std::runtime_error(
              "Sibyll target outside range. Only nuclei with A<18 or protons are "
              "allowed.");

        // beam id for sibyll
        const int kBeam = process::sibyll::ConvertToSibyllRaw(corsikaBeamId);

        std::cout << "Interaction: "
                  << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
                  << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;
        if (eProjectileLab < 8.5_GeV || Ecm < 10_GeV) {
          std::cout << "Interaction: "
                    << " DoInteraction: should have dropped particle.. "
                    << "THIS IS AN ERROR" << std::endl;
	  throw std::runtime_error("energy too low for SIBYLL");
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
          SibStack ss;

          MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
          HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
          for (auto& psib : ss) {

            // skip particles that have decayed in Sibyll
            if (psib.HasDecayed()) continue;

            // transform energy to lab. frame
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
          }
          std::cout << "conservation (all GeV): Ecm_final=" << Ecm_final / 1_GeV
                    << std::endl
                    << "Elab_final=" << Elab_final / 1_GeV
                    << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents()
                    << std::endl;
        }
      }
      return process::EProcessReturn::eOk;
    }

  private:
    corsika::environment::Environment const& fEnvironment;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  };

} // namespace corsika::process::sibyll

#endif
