/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/qgsjetII/Interaction.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/modules/qgsjetII/ParticleConversion.hpp>
#include <corsika/modules/qgsjetII/QGSJetIIFragmentsStack.hpp>
#include <corsika/modules/qgsjetII/QGSJetIIStack.hpp>
#include <corsika/modules/qgsjetII/qgsjet-II-04.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <sstream>
#include <string>
#include <tuple>


namespace corsika::qgsjetII {

  Interaction::Interaction(const std::string& dataPath)
      : data_path_(dataPath) {
    if (dataPath == "") {
      if (std::getenv("CORSIKA_DATA")) {
        data_path_ = std::string(std::getenv("CORSIKA_DATA")) + "/QGSJetII/";
        std::cout << "Searching for QGSJetII data tables in " << data_path_ << std::endl;
      }
    }
  }

  Interaction::~Interaction() { std::cout << "QgsjetII::Interaction n=" << count_ << std::endl; }

  void Interaction::Init() {

    using corsika::RNGManager;

    // initialize QgsjetII
    if (!initialized_) {
      qgset_();
      datadir DIR(data_path_);
      qgaini_(DIR.data);
      initialized_ = true;
    }
  }

  units::si::CrossSectionType Interaction::GetCrossSection(
      const corsika::Code beamId, const corsika::Code targetId,
      const units::si::HEPEnergyType Elab, const unsigned int Abeam,
      const unsigned int targetA) const {
    using namespace units::si;
    double sigProd = std::numeric_limits<double>::infinity();

    if (corsika::qgsjetII::CanInteract(beamId)) {

      const int iBeam = corsika::qgsjetII::GetQgsjetIIXSCode(beamId);
      int iTarget = 1;
      if (corsika::IsNucleus(targetId)) {
        iTarget = targetA;
        if (iTarget > maxMassNumber_ || iTarget <= 0) {
          std::ostringstream txt;
          txt << "QgsjetII target outside range. iTarget=" << iTarget;
          throw std::runtime_error(txt.str().c_str());
        }
      }
      int iProjectile = 1;
      if (corsika::IsNucleus(beamId)) {
        iProjectile = Abeam;
        if (iProjectile > maxMassNumber_ || iProjectile <= 0)
          throw std::runtime_error("QgsjetII target outside range. ");
      }

      std::cout << "QgsjetII::GetCrossSection Elab=" << Elab << " iBeam=" << iBeam
           << " iProjectile=" << iProjectile << " iTarget=" << iTarget << std::endl;
      sigProd = qgsect_(Elab / 1_GeV, iBeam, iProjectile, iTarget);
      std::cout << "QgsjetII::GetCrossSection sigProd=" << sigProd << std::endl;
    }

    return sigProd * 1_mb;
  }

  template <typename TParticle>
  units::si::GrammageType Interaction::GetInteractionLength(
  const TParticle& vP) const {

    using namespace units::si;

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    const corsika::Code corsikaBeamId = vP.GetPID();

    // beam particles for qgsjetII : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = corsika::qgsjetII::CanInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = vP.GetEnergy();

    std::cout << "Interaction: LambdaInt: \n"
         << " input energy: " << vP.GetEnergy() / 1_GeV << std::endl
         << " beam can interact:" << kInteraction << std::endl
         << " beam pid:" << vP.GetPID() << std::endl;

    if (kInteraction) {

      int Abeam = 0;
      if (corsika::IsNucleus(vP.GetPID())) Abeam = vP.GetNuclearA();

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */

      auto const* currentNode = vP.GetNode();
      const auto& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();

      units::si::CrossSectionType weightedProdCrossSection = mediumComposition.WeightedSum(
          [=](corsika::Code targetID) -> units::si::CrossSectionType {
            int targetA = 0;
            if (corsika::IsNucleus(targetID))
              targetA = corsika::GetNucleusA(targetID);
            return GetCrossSection(corsikaBeamId, targetID, Elab, Abeam, targetA);
          });

      std::cout << "Interaction: "
           << "IntLength: weighted CrossSection (mb): " << weightedProdCrossSection / 1_mb
           << std::endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      units::constants::u / weightedProdCrossSection;
      std::cout << "Interaction: "
           << "interaction length (g/cm2): " << int_length / (0.001_kg) * 1_cm * 1_cm
           << std::endl;

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function QGSJETII is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <typename TParticle>
  corsika::EProcessReturn Interaction::DoInteraction(TParticle& vP) {

    using namespace units::si;

    const auto corsikaBeamId = vP.GetPID();
    std::cout << "ProcessQgsjetII: "
         << "DoInteraction: " << corsikaBeamId << " interaction? "
         << corsika::qgsjetII::CanInteract(corsikaBeamId) << std::endl;

    if (corsika::qgsjetII::CanInteract(corsikaBeamId)) {

      const CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      // position and time of interaction, not used in QgsjetII
      Point pOrig = vP.GetPosition();
      TimeType tOrig = vP.GetTime();

      // define target
      // for QgsjetII is always a single nucleon
      // FOR NOW: target is always at rest
      const auto targetEnergyLab = 0_GeV + units::constants::nucleonMass;
      const auto targetMomentumLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(targetEnergyLab, targetMomentumLab);

      // define projectile
      HEPEnergyType const projectileEnergyLab = vP.GetEnergy();
      auto const projectileMomentumLab = vP.GetMomentum();

      int beamA = 0;
      if (corsika::IsNucleus(corsikaBeamId)) beamA = vP.GetNuclearA();

      std::cout << "Interaction: ebeam lab: " << projectileEnergyLab / 1_GeV << std::endl
           << "Interaction: pbeam lab: " << projectileMomentumLab.GetComponents() / 1_GeV
           << std::endl;
      std::cout << "Interaction: etarget lab: " << targetEnergyLab / 1_GeV << std::endl
           << "Interaction: ptarget lab: " << targetMomentumLab.GetComponents() / 1_GeV << std::endl;

      std::cout << "Interaction: position of interaction: " << pOrig.GetCoordinates() << std::endl;
      std::cout << "Interaction: time: " << tOrig << std::endl;

      // sample target mass number
      auto const* currentNode = vP.GetNode();
      auto const& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from GetInteractionLength if possible
       */
      auto const& compVec = mediumComposition.GetComponents();
      std::vector<units::si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        int targetA = 0;
        if (corsika::IsNucleus(targetId))
          targetA = corsika::GetNucleusA(targetId);
        const auto sigProd =
            GetCrossSection(corsikaBeamId, targetId, projectileEnergyLab, beamA, targetA);
        cross_section_of_components[i] = sigProd;
      }

      const auto targetCode =
          mediumComposition.SampleTarget(cross_section_of_components, fRNG);
      std::cout << "Interaction: target selected: " << targetCode << std::endl;

      int targetQgsCode = -1;
      if (corsika::IsNucleus(targetCode))
        targetQgsCode = corsika::GetNucleusA(targetCode);
      if (targetCode == corsika::Proton::GetCode()) targetQgsCode = 1;
      std::cout << "Interaction: target qgsjetII code/A: " << targetQgsCode << std::endl;
      if (targetQgsCode > maxMassNumber_ || targetQgsCode < 1)
        throw std::runtime_error("QgsjetII target outside range.");

      int projQgsCode = 1;
      if (corsika::IsNucleus(corsikaBeamId)) projQgsCode = vP.GetNuclearA();
      std::cout << "Interaction: projectile qgsjetII code/A: " << projQgsCode << " "
           << corsikaBeamId << std::endl;
      if (projQgsCode > maxMassNumber_ || projQgsCode < 1)
        throw std::runtime_error("QgsjetII target outside range.");

      // beam id for qgsjetII
      int kBeam = 2; // default: proton Shouldn't we randomize neutron/proton for nuclei?
      if (corsikaBeamId != corsika::Code::Nucleus) {
        kBeam = corsika::qgsjetII::ConvertToQgsjetIIRaw(corsikaBeamId);
        // from conex
        if (kBeam == 0) { // replace pi0 or rho0 with pi+/pi-
          static int select = 1;
          kBeam = select;
          select *= -1;
        }
        // replace lambda by neutron
        if (kBeam == 6)
          kBeam = 3;
        else if (kBeam == -6)
          kBeam = -3;
        // else if (abs(kBeam)>6) -> throw
      }

      std::cout << "Interaction: "
           << " DoInteraction: E(GeV):" << projectileEnergyLab / 1_GeV << std::endl;
      count_++;
      qgini_(projectileEnergyLab / 1_GeV, kBeam, projQgsCode, targetQgsCode);
      // this is from CRMC, is this REALLY needed ???
      qgini_(projectileEnergyLab / 1_GeV, kBeam, projQgsCode, targetQgsCode);
      qgconf_();
      
      // bookkeeping
      MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_final = 0_GeV;

      // to read the secondaries
      // define rotation to and from CoM frame
      // CoM frame definition in QgsjetII projectile: +z
      auto const& originalCS = projectileMomentumLab.GetCoordinateSystem();
      corsika::CoordinateSystem const zAxisFrame =
	originalCS.RotateToZ(projectileMomentumLab);
      
      // fragments
      QGSJetIIFragmentsStack qfs;
      for (auto& fragm : qfs) {
        corsika::Code idFragm = corsika::Code::Nucleus;
        int A = fragm.GetFragmentSize();
        int Z = 0;
        switch (A) {
          case 1: { // proton/neutron
            idFragm = corsika::Code::Proton;

	    auto momentum = corsika::Vector(
					     zAxisFrame,
					     corsika::QuantityVector<hepmomentum_d>{0.0_GeV, 0.0_GeV,
						 sqrt((projectileEnergyLab + corsika::Proton::GetMass()) *
						      (projectileEnergyLab - corsika::Proton::GetMass()))});
	    
	    auto const energy = sqrt(momentum.squaredNorm() + square(corsika::GetMass(idFragm)));	    
	    momentum.rebase(originalCS); // transform back into standard lab frame
	    std::cout << "secondary fragment> id=" << idFragm << " p=" << momentum.GetComponents() << std::endl;
            auto pnew = vP.AddSecondary(
                std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                      corsika::Point, units::si::TimeType>{
		  idFragm, energy, momentum, pOrig, tOrig});
            Plab_final += pnew.GetMomentum();
            Elab_final += pnew.GetEnergy();
          } break;
          case 2: // deuterium
            Z = 1;
            break;
          case 3: // tritium
            Z = 1;
            break;
          case 4: // helium
            Z = 2;
            break;
          default: // nucleus
          {
            Z = int(A / 2.15 + 0.7);
          }
        }

        if (idFragm == corsika::Code::Nucleus) {
	    auto momentum = corsika::Vector(
					     zAxisFrame,
					     corsika::QuantityVector<hepmomentum_d>{0.0_GeV, 0.0_GeV,
						 sqrt((projectileEnergyLab + units::constants::nucleonMass * A) *
						      (projectileEnergyLab - units::constants::nucleonMass * A))});
	    
	    auto const energy = sqrt(momentum.squaredNorm() + units::si::square(units::constants::nucleonMass*A));	    
	    momentum.rebase(originalCS); // transform back into standard lab frame
	    std::cout << "secondary fragment> id=" << idFragm << " p=" << momentum.GetComponents() << " A=" << A << " Z=" << Z << std::endl;
            auto pnew = vP.AddSecondary(
                std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
		corsika::Point, units::si::TimeType, unsigned short, unsigned short>{
		  idFragm, energy, momentum, pOrig, tOrig, A, Z});
            Plab_final += pnew.GetMomentum();
            Elab_final += pnew.GetEnergy();
        }
      }

      // secondaries
      QGSJetIIStack qs;
      for (auto& psec : qs) {

        auto momentum = psec.GetMomentum(zAxisFrame);
        auto const energy = psec.GetEnergy();

	momentum.rebase(originalCS); // transform back into standard lab frame
	std::cout << "secondary fragment> id=" << corsika::qgsjetII::ConvertFromQgsjetII(psec.GetPID()) << " p=" << momentum.GetComponents() << std::endl;
	auto pnew = vP.AddSecondary(
				    std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
				    corsika::Point, units::si::TimeType>{
		  corsika::qgsjetII::ConvertFromQgsjetII(psec.GetPID()), energy, momentum, pOrig, tOrig});
	Plab_final += pnew.GetMomentum();
	Elab_final += pnew.GetEnergy();
      }
      std::cout << "conservation (all GeV): Ecm_final= n/a" /* << Ecm_final / 1_GeV*/ << std::endl
           << "Elab_final=" << Elab_final / 1_GeV
           << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents()
           << ", N_wounded,targ="
           << QGSJetIIFragmentsStackData::GetWoundedNucleonsTarget()
           << ", N_wounded,proj="
           << QGSJetIIFragmentsStackData::GetWoundedNucleonsProjectile()
           << ", N_fragm,proj=" << qfs.GetSize() << std::endl;
    }
    return corsika::EProcessReturn::eOk;
  }

} // namespace corsika::qgsjetII
