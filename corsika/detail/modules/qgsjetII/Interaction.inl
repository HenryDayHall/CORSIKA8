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
#include <corsika/framework/utility/COMBoost.hpp>

#include <sstream>
#include <string>
#include <tuple>

#include <qgsjet-II-04.hpp>

namespace corsika::qgsjetII {

  inline Interaction::Interaction(boost::filesystem::path dataPath) {
    CORSIKA_LOG_DEBUG("Reading QGSJetII data tables from {}", dataPath);

    // initialize QgsjetII
    static bool initialized = false;
    if (!initialized) {
      qgset_();
      datadir DIR(dataPath.string() + "/");
      qgaini_(DIR.data);
      initialized = true;
    }
  }

  inline Interaction::~Interaction() {
    CORSIKA_LOG_DEBUG("QgsjetII::Interaction n= {}", count_);
  }

  inline CrossSectionType Interaction::getCrossSection(const Code beamId,
                                                       const Code targetId,
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
        if (iTarget > int(maxMassNumber_) || iTarget <= 0) {
          std::ostringstream txt;
          txt << "QgsjetII target outside range. Atarget=" << iTarget;
          throw std::runtime_error(txt.str().c_str());
        }
      }
      int iProjectile = 1;
      if (is_nucleus(beamId)) {
        iProjectile = Abeam;
        if (iProjectile > int(maxMassNumber_) || iProjectile <= 0) {
          std::ostringstream txt;
          txt << "QgsjetII projectile outside range. Aprojectile=" << iProjectile;
          throw std::runtime_error(txt.str().c_str());
        }
      }

      CORSIKA_LOG_DEBUG(
          "QgsjetII::getCrossSection Elab= {} GeV iBeam= {}"
          " iProjectile= {} iTarget= {}",
          Elab / 1_GeV, iBeam, iProjectile, iTarget);
      sigProd = qgsect_(Elab / 1_GeV, iBeam, iProjectile, iTarget);
      CORSIKA_LOG_DEBUG("QgsjetII::getCrossSection sigProd= {} mb", sigProd);
    }

    return sigProd * 1_mb;
  }

  template <typename TParticle>
  inline GrammageType Interaction::getInteractionLength(const TParticle& particle) const {

    // coordinate system, get global frame of reference
    CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();

    const Code corsikaBeamId = particle.getPID();

    // beam particles for qgsjetII : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = corsika::qgsjetII::canInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType const Elab = particle.getEnergy();

    CORSIKA_LOG_DEBUG(
        "Interaction: LambdaInt: \n"
        " input energy: {} GeV"
        " beam can interact: {}"
        " beam pid: {}",
        particle.getEnergy() / 1_GeV, kInteraction, corsikaBeamId);

    if (kInteraction) {

      int Abeam = 0;
      if (is_nucleus(corsikaBeamId)) Abeam = get_nucleus_A(corsikaBeamId);

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */

      auto const* currentNode = particle.getNode();
      const auto& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();

      CrossSectionType weightedProdCrossSection =
          mediumComposition.getWeightedSum([=](Code targetID) -> CrossSectionType {
            int targetA = 0;
            if (is_nucleus(targetID)) targetA = get_nucleus_A(targetID);
            return getCrossSection(corsikaBeamId, targetID, Elab, Abeam, targetA);
          });

      CORSIKA_LOG_DEBUG(
          "Interaction: "
          "IntLength: weighted CrossSection (mb): {}",
          weightedProdCrossSection / 1_mb);

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      CORSIKA_LOG_DEBUG(
          "Interaction: "
          "interaction length (g/cm2): {}",
          int_length / (0.001_kg) * 1_cm * 1_cm);

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function QGSJETII is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <typename TView>
  inline void Interaction::doInteraction(TView& view) {

    auto const projectile = view.getProjectile();
    auto const corsikaBeamId = projectile.getPID();
    CORSIKA_LOG_DEBUG(
        "ProcessQgsjetII: "
        "doInteraction: {} interaction possible? {}",
        corsikaBeamId, corsika::qgsjetII::canInteract(corsikaBeamId));

    if (!corsika::qgsjetII::canInteract(corsikaBeamId)) return;

    CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem();

    // position and time of interaction, not used in QgsjetII
    Point const pOrig = projectile.getPosition();
    TimeType const tOrig = projectile.getTime();

    // define target
    // for QgsjetII is always a single nucleon
    // FOR NOW: target is always at rest
    auto const targetEnergyLab = 0_GeV + constants::nucleonMass;
    auto const targetMomentumLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
    FourVector const PtargLab(targetEnergyLab, targetMomentumLab);

    // define projectile
    HEPEnergyType const projectileEnergyLab = projectile.getEnergy();
    auto const projectileMomentumLab = projectile.getMomentum();

    int beamA = 0;
    if (is_nucleus(corsikaBeamId)) beamA = get_nucleus_A(corsikaBeamId);

    HEPEnergyType const projectileEnergyLabPerNucleon = projectileEnergyLab / beamA;

    CORSIKA_LOG_DEBUG(
        "ebeam lab: {} GeV "
        "pbeam lab: {} GeV ",
        projectileEnergyLab / 1_GeV, projectileMomentumLab.getComponents() / 1_GeV);
    CORSIKA_LOG_DEBUG(
        "etarget lab: {} GeV "
        "ptarget lab: {} GeV ",
        targetEnergyLab / 1_GeV, targetMomentumLab.getComponents() / 1_GeV);
    CORSIKA_LOG_DEBUG("position of interaction: {}", pOrig.getCoordinates());
    CORSIKA_LOG_DEBUG("time: {} ", tOrig);

    // sample target mass number
    auto const* currentNode = projectile.getNode();
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

    int targetMassNumber = 1;     // proton
    if (is_nucleus(targetCode)) { // nucleus
      targetMassNumber = get_nucleus_A(targetCode);
      if (targetMassNumber > int(maxMassNumber_))
        throw std::runtime_error(
            "QgsjetII target mass outside range."); // LCOV_EXCL_LINE there is no
                                                    // allowed path here
    } else {
      if (targetCode != Proton::code) // LCOV_EXCL_LINE there is no allowed path here
        throw std::runtime_error(
            "QgsjetII Taget not possible."); // LCOV_EXCL_LINE there is no allowed path
                                             // here
    }
    CORSIKA_LOG_DEBUG("target: {}, qgsjetII code/A: {}", targetCode, targetMassNumber);

    int projectileMassNumber = 1; // "1" means "hadron"
    QgsjetIIHadronType qgsjet_hadron_type =
        qgsjetII::getQgsjetIIHadronType(corsikaBeamId);
    if (qgsjet_hadron_type == QgsjetIIHadronType::NucleusType) {
      projectileMassNumber = get_nucleus_A(corsikaBeamId);
      if (projectileMassNumber > int(maxMassNumber_))
        throw std::runtime_error(
            "QgsjetII projectile mass outside range."); // LCOV_EXCL_LINE there is no
                                                        // allowed path here
      std::array<QgsjetIIHadronType, 2> constexpr nucleons = {
          QgsjetIIHadronType::ProtonType, QgsjetIIHadronType::NeutronType};
      std::uniform_int_distribution select(0, 1);
      qgsjet_hadron_type = nucleons[select(rng_)];
    } else {
      // from conex: replace pi0 or rho0 with pi+/pi- in alternating sequence
      if (qgsjet_hadron_type == QgsjetIIHadronType::NeutralLightMesonType) {
        qgsjet_hadron_type = alternate_;
        alternate_ = (alternate_ == QgsjetIIHadronType::PiPlusType
                          ? QgsjetIIHadronType::PiMinusType
                          : QgsjetIIHadronType::PiPlusType);
      }
    }

    // beam id for qgsjetII
    int kBeam = 2; // default: proton Shouldn't we randomize neutron/proton for nuclei?
    if (!is_nucleus(corsikaBeamId)) {
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

    count_++;
    int qgsjet_hadron_type_int = static_cast<QgsjetIICodeIntType>(qgsjet_hadron_type);
    CORSIKA_LOG_DEBUG(
        "qgsjet_hadron_type_int={} projectileMassNumber={} targetMassNumber={}",
        qgsjet_hadron_type_int, projectileMassNumber, targetMassNumber);
    qgini_(projectileEnergyLab / 1_GeV, qgsjet_hadron_type_int, projectileMassNumber,
           targetMassNumber);
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
      int const A = fragm.getFragmentSize();
      if (A == 1) { // nucleon
        std::uniform_real_distribution<double> select;
        Code idFragm = Code::Proton;
        if (select(rng_) > 0.5) { idFragm = Code::Neutron; }

        const HEPMassType nucleonMass = get_mass(idFragm);
        // no pT, frgments just go forward
        auto momentum =
            Vector(zAxisFrame, corsika::QuantityVector<hepmomentum_d>{
                                   0.0_GeV, 0.0_GeV,
                                   sqrt((projectileEnergyLabPerNucleon + nucleonMass) *
                                        (projectileEnergyLabPerNucleon - nucleonMass))});

        momentum.rebase(originalCS); // transform back into standard lab frame
        CORSIKA_LOG_DEBUG(
            "secondary fragment> id= {}"
            " p={}",
            idFragm, momentum.getComponents());
        auto pnew = view.addSecondary(std::make_tuple(idFragm, momentum, pOrig, tOrig));
        Plab_final += pnew.getMomentum();
        Elab_final += pnew.getEnergy();

      } else { // nucleus, A>1

        int Z = 0;
        switch (A) {
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

        HEPMassType const nucleusMass = Proton::mass * Z + Neutron::mass * (A - Z);
        // no pT, frgments just go forward
        auto momentum = Vector(
            zAxisFrame, QuantityVector<hepmomentum_d>{
                            0.0_GeV, 0.0_GeV,
                            sqrt((projectileEnergyLabPerNucleon * A + nucleusMass) *
                                 (projectileEnergyLabPerNucleon * A - nucleusMass))});

        momentum.rebase(originalCS); // transform back into standard lab frame
        CORSIKA_LOG_DEBUG(
            "secondary fragment> id={}"
            " p={}"
            " A={}"
            " Z={}",
            get_nucleus_code(A, Z), momentum.getComponents(), A, Z);

        auto pnew = view.addSecondary(
            std::make_tuple(get_nucleus_code(A, Z), momentum, pOrig, tOrig));
        Plab_final += pnew.getMomentum();
        Elab_final += pnew.getEnergy();
      }
    }

    // secondaries
    QGSJetIIStack qs;
    for (auto& psec : qs) {

      auto momentum = psec.getMomentum(zAxisFrame);

      momentum.rebase(originalCS); // transform back into standard lab frame
      CORSIKA_LOG_DEBUG("secondary> id= {}, p= {}",
                        corsika::qgsjetII::convertFromQgsjetII(psec.getPID()),
                        momentum.getComponents());
      auto pnew = view.addSecondary(std::make_tuple(
          corsika::qgsjetII::convertFromQgsjetII(psec.getPID()), momentum, pOrig, tOrig));
      Plab_final += pnew.getMomentum();
      Elab_final += pnew.getEnergy();
    }
    CORSIKA_LOG_DEBUG(
        "conservation (all GeV): Ecm_final= n/a " /* << Ecm_final / 1_GeV*/
        ", Elab_final={} "
        ", Plab_final={}"
        ", N_wounded,targ={}"
        ", N_wounded,proj={}"
        ", N_fragm,proj={}",
        Elab_final / 1_GeV, (Plab_final / 1_GeV).getComponents(),
        QGSJetIIFragmentsStackData::getWoundedNucleonsTarget(),
        QGSJetIIFragmentsStackData::getWoundedNucleonsProjectile(), qfs.getSize());
  }
} // namespace corsika::qgsjetII
