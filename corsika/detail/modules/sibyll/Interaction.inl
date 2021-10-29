/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/Interaction.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/modules/sibyll/ParticleConversion.hpp>
#include <corsika/modules/sibyll/SibStack.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <sibyll2.3d.hpp>

#include <tuple>

namespace corsika::sibyll {

  inline Interaction::Interaction(const bool sibyll_printout_on)
      : sibyll_listing_(sibyll_printout_on) {
    // initialize Sibyll
    static bool initialized = false;
    if (!initialized) {
      sibyll_ini_();
      initialized = true;
    }
  }

  inline Interaction::~Interaction() {
    CORSIKA_LOG_DEBUG("Sibyll::Interaction n={}, Nnuc={}", count_, nucCount_);
  }

  inline std::tuple<corsika::CrossSectionType, corsika::CrossSectionType>
  Interaction::getCrossSection(const corsika::Code BeamId, const corsika::Code TargetId,
                               const corsika::HEPEnergyType CoMenergy) const {
    double sigProd, sigEla, dummy, dum1, dum3, dum4;
    double dumdif[3];
    const int iBeam = corsika::sibyll::getSibyllXSCode(
        BeamId); // 0 (can not interact, 1: proton-like, 2: pion-like, 3:kaon-like)
    if (!iBeam)
      throw std::runtime_error(
          fmt::format("Interaction of beam {} not defined in "
                      "Sibyll!",
                      BeamId));
    if (!isValidCoMEnergy(CoMenergy)) {
      throw std::runtime_error(
          "Interaction: getCrossSection: CoM energy outside range for Sibyll!");
    }
    const double dEcm = CoMenergy / 1_GeV;
    // single nucleon target (p,n, hydrogen) or 4<=A<=18
    if (isValidTarget(TargetId)) {
      // single nucleon target
      if (TargetId == corsika::Code::Proton || TargetId == Code::Hydrogen ||
          TargetId == Code::Neutron) {
        sib_sigma_hp_(iBeam, dEcm, dum1, sigEla, sigProd, dumdif, dum3, dum4);
      } else {
        // nuclear target
        const int iTarget = corsika::get_nucleus_A(TargetId);
        sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy, sigEla);
      }
    } else {
      //         throw std::runtime_error(
      //            "Sibyll nuclear target outside range. Only nuclei with 4<=A<18 are
      //            allowed.");

      // no interaction in sibyll possible, return infinite cross section? or throw?
      sigProd = std::numeric_limits<double>::infinity();
      sigEla = std::numeric_limits<double>::infinity();
    }
    return std::make_tuple(sigProd * 1_mb, sigEla * 1_mb);
  }

  template <typename TParticle>
  inline corsika::GrammageType Interaction::getInteractionLength(
      TParticle const& projectile) const {

    const corsika::Code corsikaBeamId = projectile.getPID();

    // beam corsika for sibyll : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = corsika::sibyll::canInteract(corsikaBeamId);

    MomentumVector const& pLab = projectile.getMomentum();
    CoordinateSystemPtr const& labCS = pLab.getCoordinateSystem();

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(labCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = projectile.getEnergy() + constants::nucleonMass;
    MomentumVector pTotLab(labCS, {0_GeV, 0_GeV, 0_GeV});
    pTotLab += pLab;
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.getNorm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

    CORSIKA_LOG_DEBUG(
        "Interaction: LambdaInt: \n"
        " input energy: {} GeV "
        " beam can interact: {} "
        " beam pid: {}",
        projectile.getEnergy() / 1_GeV, kInteraction, projectile.getPID());

    // TODO: move limits into variables
    // FR: removed && Elab >= 8.5_GeV
    if (kInteraction && isValidCoMEnergy(ECoM)) {

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */

      auto const* currentNode = projectile.getNode();
      const auto& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();

      si::CrossSectionType weightedProdCrossSection = mediumComposition.getWeightedSum(
          [=](corsika::Code targetID) -> si::CrossSectionType {
            // Argon needs special handling ....
            return targetID == Code::Argon ? CrossSectionType::zero()
                                           : std::get<0>(this->getCrossSection(
                                                 corsikaBeamId, targetID, ECoM));
          });

      CORSIKA_LOG_DEBUG(
          "Interaction: "
          "IntLength: weighted CrossSection (mb): {} ",
          weightedProdCrossSection / 1_mb);

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      CORSIKA_LOG_DEBUG(
          "Interaction: "
          "interaction length (g/cm2): {} ",
          int_length / (0.001_kg) * 1_cm * 1_cm);

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function SIBYLL is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <typename TSecondaryView>
  inline void Interaction::doInteraction(TSecondaryView& view) {

    auto const projectile = view.getProjectile();
    const auto corsikaBeamId = projectile.getPID();

    if (corsika::is_nucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by SIBYLL!");
    }

    // position and time of interaction, not used in Sibyll
    Point const pOrig = projectile.getPosition();
    TimeType const tOrig = projectile.getTime();

    // define projectile
    HEPEnergyType const eProjectileLab = projectile.getEnergy();
    auto const pProjectileLab = projectile.getMomentum();
    CoordinateSystemPtr const& originalCS = pProjectileLab.getCoordinateSystem();

    CORSIKA_LOG_DEBUG(
        "ProcessSibyll: "
        "DoInteraction: pid {} interaction ",
        corsikaBeamId);

    // define target
    // for Sibyll is always a single nucleon
    // FOR NOW: target is always at rest
    const auto eTargetLab = 0_GeV + constants::nucleonMass;
    const auto pTargetLab = MomentumVector(originalCS, 0_GeV, 0_GeV, 0_GeV);
    const FourVector PtargLab(eTargetLab, pTargetLab);

    CORSIKA_LOG_DEBUG(
        "Interaction: ebeam lab: {} GeV"
        "Interaction: pbeam lab: {} GeV",
        eProjectileLab / 1_GeV, pProjectileLab.getComponents());
    CORSIKA_LOG_DEBUG(
        "Interaction: etarget lab: {} GeV "
        "Interaction: ptarget lab: {} GeV",
        eTargetLab / 1_GeV, pTargetLab.getComponents() / 1_GeV);

    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    // define target kinematics in lab frame
    // define boost to and from CoM frame
    // CoM frame definition in Sibyll projectile: +z
    COMBoost const boost(PprojLab, constants::nucleonMass);
    auto const& csPrime = boost.getRotatedCS();

    // just for show:
    // boost projecticle
    [[maybe_unused]] auto const PprojCoM = boost.toCoM(PprojLab);
    // boost target
    [[maybe_unused]] auto const PtargCoM = boost.toCoM(PtargLab);
    CORSIKA_LOG_DEBUG(
        "Interaction: ebeam CoM: {} GeV "
        "Interaction: pbeam CoM: {} GeV ",
        PprojCoM.getTimeLikeComponent() / 1_GeV,
        PprojCoM.getSpaceLikeComponents().getComponents(csPrime) / 1_GeV);
    CORSIKA_LOG_DEBUG(
        "Interaction: etarget CoM: {} GeV "
        "Interaction: ptarget CoM: {} GeV ",
        PtargCoM.getTimeLikeComponent() / 1_GeV,
        PtargCoM.getSpaceLikeComponents().getComponents(csPrime) / 1_GeV);

    CORSIKA_LOG_DEBUG("Interaction: position of interaction: {} ",
                      pOrig.getCoordinates());
    CORSIKA_LOG_DEBUG("Interaction: time: {} ", tOrig);

    HEPEnergyType Etot = eProjectileLab + eTargetLab;
    MomentumVector Ptot = projectile.getMomentum();
    // invariant mass, i.e. cm. energy
    HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.getSquaredNorm());

    // sample target mass number
    auto const* currentNode = projectile.getNode();
    auto const& mediumComposition =
        currentNode->getModelProperties().getNuclearComposition();
    // get cross sections for target materials
    /*
      Here we read the cross section from the interaction model again,
      should be passed from getInteractionLength if possible
     */
    //#warning reading interaction cross section again, should not be necessary
    auto const& compVec = mediumComposition.getComponents();
    std::vector<CrossSectionType> cross_section_of_components(compVec.size());

    for (size_t i = 0; i < compVec.size(); ++i) {
      auto const targetId = compVec[i];
      if (targetId == Code::Argon) continue; // skip Argon ....
      const auto [sigProd, sigEla] = getCrossSection(corsikaBeamId, targetId, Ecm);
      [[maybe_unused]] const auto& dummy_sigEla = sigEla;
      cross_section_of_components[i] = sigProd;
    }

    const auto targetCode =
        mediumComposition.sampleTarget(cross_section_of_components, RNG_);
    CORSIKA_LOG_DEBUG("Interaction: target selected: {} ", targetCode);
    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int targetSibCode = -1;
    if (is_nucleus(targetCode)) targetSibCode = get_nucleus_A(targetCode);
    if (targetCode == Proton::code) targetSibCode = 1;
    CORSIKA_LOG_DEBUG("Interaction: sibyll code: {}", targetSibCode);
    if (targetSibCode > int(maxTargetMassNumber_) || targetSibCode < 1)
      throw std::runtime_error(
          "Sibyll target outside range. Only nuclei with A<18 or protons are "
          "allowed.");

    // beam id for sibyll
    const int kBeam = corsika::sibyll::convertToSibyllRaw(corsikaBeamId);

    CORSIKA_LOG_DEBUG(
        "Interaction: "
        " DoInteraction: E(GeV): {} "
        " Ecm(GeV): {} ",
        eProjectileLab / 1_GeV, Ecm / 1_GeV);
    if (Ecm > getMaxEnergyCoM())
      throw std::runtime_error("Interaction::DoInteraction: CoM energy too high!");
    // FR: removed eProjectileLab < 8.5_GeV ||
    if (Ecm < getMinEnergyCoM()) {
      CORSIKA_LOG_DEBUG(
          "Interaction: "
          " DoInteraction: should have dropped particle.. "
          "THIS IS AN ERROR");
      throw std::runtime_error("energy too low for SIBYLL");
    } else {
      count_++;
      // Sibyll does not know about units..
      const double sqs = Ecm / 1_GeV;
      // running sibyll, filling stack
      sibyll_(kBeam, targetSibCode, sqs);

      if (sibyll_listing_) {
        // print final state
        int print_unit = 6;
        sib_list_(print_unit);
        nucCount_ += get_nwounded() - 1;
      }

      // add particles from sibyll to stack
      // link to sibyll stack
      SibStack ss;

      MomentumVector Plab_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
      for (auto& psib : ss) {

        // abort on particles that have decayed in Sibyll. Should not happen!
        if (psib.hasDecayed())
          throw std::runtime_error("found particle that decayed in SIBYLL!");

        // transform 4-momentum to lab. frame
        // note that the momentum needs to be rotated back
        auto const tmp = psib.getMomentum().getComponents();
        auto const pCoM = Vector<hepmomentum_d>(csPrime, tmp);
        HEPEnergyType const eCoM = psib.getEnergy();
        auto const Plab = boost.fromCoM(FourVector(eCoM, pCoM));
        auto const p3lab = Plab.getSpaceLikeComponents();
        assert(p3lab.getCoordinateSystem() == originalCS); // just to be sure!

        // add to corsika stack
        auto pnew = view.addSecondary(std::make_tuple(
            corsika::sibyll::convertFromSibyll(psib.getPID()), p3lab, pOrig, tOrig));

        Plab_final += pnew.getMomentum();
        Elab_final += pnew.getEnergy();
        Ecm_final += psib.getEnergy();
      }
      CORSIKA_LOG_DEBUG(
          "conservation (all GeV): "
          "Ecm_initial(per nucleon)={:.2f}, Ecm_final(per nucleon)={:.2f}, "
          "Elab_initial={:.2f}, Elab_final={:.2f}, "
          "Elab-diff (%)={:.2f}, "
          "m in target nucleons={:.2f}, "
          "Plab_initial={:.2f}, "
          "Plab_final={:.2f} ",
          Ecm / 1_GeV, Ecm_final * 2. / (get_nwounded() + 1) / 1_GeV, Etot / 1_GeV,
          Elab_final / 1_GeV,
          (Elab_final / (Etot + get_nwounded() * constants::nucleonMass) - 1) * 100,
          constants::nucleonMass * get_nwounded() / 1_GeV,
          (pProjectileLab / 1_GeV).getComponents(), (Plab_final / 1_GeV).getComponents());
    }
  }

} // namespace corsika::sibyll
