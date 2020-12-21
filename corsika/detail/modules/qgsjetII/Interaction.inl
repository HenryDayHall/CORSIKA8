/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <sstream>
#include <string>
#include <tuple>

#include <qgsjet-II-04.hpp>

namespace corsika::qgsjetII {

  Interaction::Interaction(const std::string& dataPath)
      : data_path_(dataPath) {
    if (dataPath == "") {
      if (std::getenv("CORSIKA_DATA")) {
        data_path_ = std::string(std::getenv("CORSIKA_DATA")) + "/QGSJetII/";
        std::cout << "Searching for QGSJetII data tables in " << data_path_ << std::endl;
      }
    }

    // initialize QgsjetII
    static bool initialized = false;
    if (!initialized) {
      qgset_();
      datadir DIR(data_path_);
      qgaini_(DIR.data);
      initialized = true;
    }
  }

  Interaction::~Interaction() {
    std::cout << "QgsjetII::Interaction n=" << count_ << std::endl;
  }

  CrossSectionType Interaction::getCrossSection(const Code beamId, const Code targetId,
                                                const HEPEnergyType Elab,
                                                const unsigned int Abeam,
                                                const unsigned int targetA) const {
    double sigProd = std::numeric_limits<double>::infinity();

    if (corsika::qgsjetII::canInteract(beamId)) {

      int const iBeam = static_cast<QgsjetIIXSClassIntType>(
          corsika::qgsjetII::getQgsjetIIXSCode(beamId));
      int iTarget = 1;
      if (is_nucleus(targetId)) {
        iTarget = targetA;
        if (iTarget > maxMassNumber_ || iTarget <= 0) {
          std::ostringstream txt;
          txt << "QgsjetII target outside range. iTarget=" << iTarget;
          throw std::runtime_error(txt.str().c_str());
        }
      }
      int iProjectile = 1;
      if (is_nucleus(beamId)) {
        iProjectile = Abeam;
        if (iProjectile > maxMassNumber_ || iProjectile <= 0)
          throw std::runtime_error("QgsjetII target outside range. ");
      }

      std::cout << "QgsjetII::getCrossSection Elab=" << Elab << " iBeam=" << iBeam
                << " iProjectile=" << iProjectile << " iTarget=" << iTarget << std::endl;
      sigProd = qgsect_(Elab / 1_GeV, iBeam, iProjectile, iTarget);
      std::cout << "QgsjetII::getCrossSection sigProd=" << sigProd << std::endl;
    }

    return sigProd * 1_mb;
  }

  template <typename TParticle>
  GrammageType Interaction::getInteractionLength(const TParticle& vP) const {

    // coordinate system, get global frame of reference
    CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();

    const Code corsikaBeamId = vP.getPID();

    // beam particles for qgsjetII : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = corsika::qgsjetII::canInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = vP.getEnergy();

    std::cout << "Interaction: LambdaInt: \n"
              << " input energy: " << vP.getEnergy() / 1_GeV << std::endl
              << " beam can interact:" << kInteraction << std::endl
              << " beam pid:" << vP.getPID() << std::endl;

    if (kInteraction) {

      int Abeam = 0;
      if (is_nucleus(vP.getPID())) Abeam = vP.getNuclearA();

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */

      auto const* currentNode = vP.getNode();
      const auto& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();

      CrossSectionType weightedProdCrossSection =
          mediumComposition.getWeightedSum([=](Code targetID) -> CrossSectionType {
            int targetA = 0;
            if (is_nucleus(targetID)) targetA = get_nucleus_A(targetID);
            return getCrossSection(corsikaBeamId, targetID, Elab, Abeam, targetA);
          });

      std::cout << "Interaction: "
                << "IntLength: weighted CrossSection (mb): "
                << weightedProdCrossSection / 1_mb << std::endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
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
  void Interaction::doInteraction(TParticle& vP) {

    const auto corsikaBeamId = vP.getPID();
    std::cout << "ProcessQgsjetII: "
              << "DoInteraction: " << corsikaBeamId << " interaction? "
              << corsika::qgsjetII::canInteract(corsikaBeamId) << std::endl;

    if (corsika::qgsjetII::canInteract(corsikaBeamId)) {

      CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();

      // position and time of interaction, not used in QgsjetII
      Point pOrig = vP.getPosition();
      TimeType tOrig = vP.getTime();

      // define target
      // for QgsjetII is always a single nucleon
      // FOR NOW: target is always at rest
      const auto targetEnergyLab = 0_GeV + constants::nucleonMass;
      const auto targetMomentumLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(targetEnergyLab, targetMomentumLab);

      // define projectile
      HEPEnergyType const projectileEnergyLab = vP.getEnergy();
      auto const projectileMomentumLab = vP.getMomentum();

      int beamA = 0;
      if (is_nucleus(corsikaBeamId)) beamA = vP.getNuclearA();

      std::cout << "Interaction: ebeam lab: " << projectileEnergyLab / 1_GeV << std::endl
                << "Interaction: pbeam lab: "
                << projectileMomentumLab.getComponents() / 1_GeV << std::endl;
      std::cout << "Interaction: etarget lab: " << targetEnergyLab / 1_GeV << std::endl
                << "Interaction: ptarget lab: "
                << targetMomentumLab.getComponents() / 1_GeV << std::endl;

      std::cout << "Interaction: position of interaction: " << pOrig.getCoordinates()
                << std::endl;
      std::cout << "Interaction: time: " << tOrig << std::endl;

      // sample target mass number
      auto const* currentNode = vP.getNode();
      auto const& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from getInteractionLength if possible
       */
      auto const& compVec = mediumComposition.getComponents();
      std::vector<CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        int targetA = 0;
        if (is_nucleus(targetId)) targetA = get_nucleus_A(targetId);
        const auto sigProd =
            getCrossSection(corsikaBeamId, targetId, projectileEnergyLab, beamA, targetA);
        cross_section_of_components[i] = sigProd;
      }

      const auto targetCode =
          mediumComposition.sampleTarget(cross_section_of_components, rng_);
      std::cout << "Interaction: target selected: " << targetCode << std::endl;

      int targetQgsCode = -1;
      if (is_nucleus(targetCode)) targetQgsCode = get_nucleus_A(targetCode);
      if (targetCode == Code::Proton) targetQgsCode = 1;
      std::cout << "Interaction: target qgsjetII code/A: " << targetQgsCode << std::endl;
      if (targetQgsCode > maxMassNumber_ || targetQgsCode < 1)
        throw std::runtime_error("QgsjetII target outside range.");

      int projQgsCode = 1;
      if (is_nucleus(corsikaBeamId)) projQgsCode = vP.getNuclearA();
      std::cout << "Interaction: projectile qgsjetII code/A: " << projQgsCode << " "
                << corsikaBeamId << std::endl;
      if (projQgsCode > maxMassNumber_ || projQgsCode < 1)
        throw std::runtime_error("QgsjetII target outside range.");

      // beam id for qgsjetII
      int kBeam = 2; // default: proton Shouldn't we randomize neutron/proton for nuclei?
      if (corsikaBeamId != Code::Nucleus) {
        kBeam = corsika::qgsjetII::convertToQgsjetIIRaw(corsikaBeamId);
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
      auto const& originalCS = projectileMomentumLab.getCoordinateSystem();
      CoordinateSystemPtr const zAxisFrame =
          make_rotationToZ(originalCS, projectileMomentumLab);

      // fragments
      QGSJetIIFragmentsStack qfs;
      for (auto& fragm : qfs) {
        Code idFragm = Code::Nucleus;
        int A = fragm.getFragmentSize();
        int Z = 0;
        switch (A) {
          case 1: { // proton/neutron
            idFragm = Code::Proton;

            auto momentum =
                Vector(zAxisFrame, QuantityVector<hepmomentum_d>{
                                       0.0_GeV, 0.0_GeV,
                                       sqrt((projectileEnergyLab + Proton::mass) *
                                            (projectileEnergyLab - Proton::mass))});

            auto const energy =
                sqrt(momentum.getSquaredNorm() + square(get_mass(idFragm)));
            momentum.rebase(originalCS); // transform back into standard lab frame
            std::cout << "secondary fragment> id=" << idFragm
                      << " p=" << momentum.getComponents() << std::endl;
            auto pnew =
                vP.addSecondary(std::make_tuple(idFragm, energy, momentum, pOrig, tOrig));
            Plab_final += pnew.getMomentum();
            Elab_final += pnew.getEnergy();
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

        if (idFragm == Code::Nucleus) {
          auto momentum = Vector(
              zAxisFrame, QuantityVector<hepmomentum_d>{
                              0.0_GeV, 0.0_GeV,
                              sqrt((projectileEnergyLab + constants::nucleonMass * A) *
                                   (projectileEnergyLab - constants::nucleonMass * A))});

          auto const energy =
              sqrt(momentum.getSquaredNorm() + square(constants::nucleonMass * A));
          momentum.rebase(originalCS); // transform back into standard lab frame
          std::cout << "secondary fragment> id=" << idFragm
                    << " p=" << momentum.getComponents() << " A=" << A << " Z=" << Z
                    << std::endl;
          auto pnew = vP.addSecondary(
              std::make_tuple(idFragm, energy, momentum, pOrig, tOrig, A, Z));
          Plab_final += pnew.getMomentum();
          Elab_final += pnew.getEnergy();
        }
      }

      // secondaries
      QGSJetIIStack qs;
      for (auto& psec : qs) {

        auto momentum = psec.getMomentum(zAxisFrame);
        auto const energy = psec.getEnergy();

        momentum.rebase(originalCS); // transform back into standard lab frame
        std::cout << "secondary fragment> id="
                  << corsika::qgsjetII::convertFromQgsjetII(psec.getPID())
                  << " p=" << momentum.getComponents() << std::endl;
        auto pnew = vP.addSecondary(
            std::make_tuple(corsika::qgsjetII::convertFromQgsjetII(psec.getPID()), energy,
                            momentum, pOrig, tOrig));
        Plab_final += pnew.getMomentum();
        Elab_final += pnew.getEnergy();
      }
      std::cout << "conservation (all GeV): Ecm_final= n/a" /* << Ecm_final / 1_GeV*/
                << std::endl
                << "Elab_final=" << Elab_final / 1_GeV
                << ", Plab_final=" << (Plab_final / 1_GeV).getComponents()
                << ", N_wounded,targ="
                << QGSJetIIFragmentsStackData::getWoundedNucleonsTarget()
                << ", N_wounded,proj="
                << QGSJetIIFragmentsStackData::getWoundedNucleonsProjectile()
                << ", N_fragm,proj=" << qfs.getSize() << std::endl;
    }
  }

} // namespace corsika::qgsjetII
