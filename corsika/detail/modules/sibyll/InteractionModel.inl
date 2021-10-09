/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/FourVector.hpp>

#include <corsika/modules/sibyll/ParticleConversion.hpp>
#include <corsika/modules/sibyll/SibStack.hpp>

#include <sibyll2.3d.hpp>

#include <tuple>

namespace corsika::sibyll {

  inline void InteractionModel::setVerbose(bool const flag) { sibyll_listing_ = flag; }

  inline InteractionModel::InteractionModel()
      : sibyll_listing_(false) {
    // initialize Sibyll
    static bool initialized = false;
    if (!initialized) {
      sibyll_ini_();
      initialized = true;
    }
  }

  inline InteractionModel::~InteractionModel() {
    CORSIKA_LOG_DEBUG("Sibyll::Model n={}, Nnuc={}", count_, nucCount_);
  }

  inline void constexpr InteractionModel::isValid(Code const projectileId,
                                                  Code const targetId,
                                                  HEPEnergyType const sqrtSnn,
                                                  unsigned int const,
                                                  unsigned int const targetA) const {
    if ((minEnergyCoM_ > sqrtSnn) || (sqrtSnn > maxEnergyCoM_)) {
      // i.e. nuclei handled by different process, this should not happen
      throw std::runtime_error("CoM energy out of bounds for SIBYLL");
    }

    unsigned int targA = targetA;
    if (is_nucleus(targetId) && targetId != Code::Nucleus) {
      targA = get_nucleus_A(targetId);
    }

    if (is_nucleus(targetId)) {
      if (targA != 1 && (targA < minNuclearTargetA_ || targA >= maxTargetMassNumber_)) {
        throw std::runtime_error("Target outside of allowed range for SIBYLL");
      }
    } else if (targetId != Code::Proton && targetId != Code::Neutron) {
      throw std::runtime_error("Target cannot be handled by SIBYLL");
    }
    if (is_nucleus(projectileId) || !corsika::sibyll::canInteract(projectileId)) {
      throw std::runtime_error("Projectile cannot be handled by SIBYLL");
    }
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  InteractionModel::getCrossSectionInelEla(Code const projectileId, Code const targetId,
                                           HEPEnergyType const sqrtSnn,
                                           unsigned int const projectileA,
                                           unsigned int const targetA) const {

    isValid(projectileId, targetId, sqrtSnn, projectileA, targetA); // throws

    double dummy, dum1, dum3, dum4, dumdif[3]; // dummies needed for fortran call
    int const iBeam = corsika::sibyll::getSibyllXSCode(
        projectileId); // 0 (can not interact, 1: proton-like, 2: pion-like,
                       // 3:kaon-like)

    double const dEcm = sqrtSnn / 1_GeV;
    // single nucleon target (p,n, hydrogen) or 4<=A<=18
    double sigProd = 0;
    double sigEla = 0;
    // single nucleon target
    if (targetId == Code::Proton || targetId == Code::Hydrogen ||
        targetId == Code::Neutron) {
      sib_sigma_hp_(iBeam, dEcm, dum1, sigEla, sigProd, dumdif, dum3, dum4);
    } else {
      // nuclear target
      int const iTarget = targetA;
      sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy, sigEla);
    }
    return {sigProd * 1_mb, sigEla * 1_mb};
  } // namespace corsika::sibyll

  /**
   * In this function SIBYLL is called to produce one event. The
   * event is copied (and boosted) into the shower lab frame.
   */

  template <typename TSecondaryView>
  inline void InteractionModel::doInteraction(
      TSecondaryView& secondaries, COMBoost const& boost, Code const projectileId,
      Code const targetId, HEPEnergyType const sqrtSnn, unsigned int const projectileA,
      unsigned int const targetA) {

    isValid(projectileId, targetId, sqrtSnn, projectileA, targetA); // throws

    CORSIKA_LOG_DEBUG("pId={} tId={} sqrtSnn={}GeV", projectileId, targetId, sqrtSnn);

    int targetSibCode = -1;
    if (is_nucleus(targetId)) { targetSibCode = targetA; }
    if (targetId == Proton::code) targetSibCode = 1;
    CORSIKA_LOG_DEBUG("sibyll code: {}", targetSibCode);

    // beam id for sibyll
    int const projectileSibyllCode = corsika::sibyll::convertToSibyllRaw(projectileId);

    count_++;
    // Sibyll does not know about units..
    double const sqs = sqrtSnn / 1_GeV;
    // running sibyll, filling stack
    sibyll_(projectileSibyllCode, targetSibCode, sqs);

    if (sibyll_listing_) {
      // print final state
      int print_unit = 6;
      sib_list_(print_unit);
      nucCount_ += get_nwounded() - 1;
    }

    // ------ output and particle readout -----
    auto const& csPrime = boost.getRotatedCS();

    // add particles from sibyll to stack

    // position and time of interaction, not used in Sibyll
    Point const pOrig = Point(csPrime, {0_m, 0_m, 0_m});
    TimeType const tOrig = 0_s; // no time in sibyll
    CORSIKA_LOG_DEBUG("position of interaction: {}, time {} ", pOrig.getCoordinates(),
                      tOrig);

    // link to sibyll stack
    SibStack ss;

    auto const& originalCS = boost.getOriginalCS();
    MomentumVector Plab_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
    for (auto& psib : ss) {
      // abort on particles that have decayed in Sibyll. Should not happen!
      if (psib.hasDecayed())
        throw std::runtime_error("found particle that decayed in SIBYLL!");

      // transform 4-momentum to lab. frame
      // note that the momentum needs to be rotated back
      auto const tmp = psib.getMomentum().getComponents();
      auto const pCoM = MomentumVector(csPrime, tmp);
      HEPEnergyType const eCoM = psib.getEnergy();
      auto const Plab = boost.fromCoM(FourVector{eCoM, pCoM});
      auto const p3lab = Plab.getSpaceLikeComponents();

      // add to corsika stack
      auto pnew = secondaries.addSecondary(std::make_tuple(
          corsika::sibyll::convertFromSibyll(psib.getPID()), p3lab, pOrig, tOrig));

      Plab_final += pnew.getMomentum();
      Elab_final += pnew.getEnergy();
      Ecm_final += psib.getEnergy();
    }
    HEPEnergyType const Elab_initial =
        static_pow<2>(sqrtSnn) / (2 * constants::nucleonMass);
    CORSIKA_LOG_DEBUG(
        "conservation (all GeV): "
        "sqrtSnn={}, sqrtSnn_final={}, "
        "Elab_initial={}, Elab_final={}, "
        "diff(%)={}, "
        "E in nucleons={}, "
        "Plab_final={} ",
        sqrtSnn / 1_GeV, Ecm_final * 2. / (get_nwounded() + 1) / 1_GeV, Elab_initial,
        Elab_final / 1_GeV, (Elab_final - Elab_initial) / Elab_initial * 100,
        constants::nucleonMass * get_nwounded() / 1_GeV,
        (Plab_final / 1_GeV).getComponents());
  }

} // namespace corsika::sibyll
