/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>

#include <corsika/modules/sibyll/ParticleConversion.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/modules/sibyll/SibStack.hpp>

#include <sibyll2.3d.hpp>

#include <tuple>

namespace corsika::sibyll {

  inline void HadronInteractionModel::setVerbose(bool const flag) { sibyll_listing_ = flag; }

  inline HadronInteractionModel::HadronInteractionModel()
      : sibyll_listing_(false) {
    // initialize Sibyll
    static bool initialized = false;
    if (!initialized) {
      sibyll_ini_();
      initialized = true;
    }
  }

  inline HadronInteractionModel::~HadronInteractionModel() {
    CORSIKA_LOG_DEBUG("Sibyll::Model n={}, Nnuc={}", count_, nucCount_);
  }

  inline bool constexpr HadronInteractionModel::isValid(Code const projectileId,
                                                  Code const targetId,
                                                  HEPEnergyType const sqrtSnn) const {
    if ((minEnergyCoM_ > sqrtSnn) || (sqrtSnn > maxEnergyCoM_)) { return false; }

    if (is_nucleus(targetId)) {
      size_t const targA = get_nucleus_A(targetId);
      if (targA != 1 && (targA < minNuclearTargetA_ || targA >= maxTargetMassNumber_)) {
        return false;
      }
    } else if (targetId != Code::Proton && targetId != Code::Neutron &&
               targetId != Code::Hydrogen) {
      return false;
    }
    if (is_nucleus(projectileId) || !corsika::sibyll::canInteract(projectileId)) {
      return false;
    }
    return true;
  }

  inline std::tuple<CrossSectionType, CrossSectionType>
  HadronInteractionModel::getCrossSectionInelEla(Code const projectileId, Code const targetId,
                                           FourMomentum const& projectileP4,
                                           FourMomentum const& targetP4) const {

    int targetSibCode = 1; // nucleon or particle count
    if (is_nucleus(targetId)) { targetSibCode = get_nucleus_A(targetId); }
    // sqrtS per target nucleon
    HEPEnergyType const sqrtSnn = (projectileP4 + targetP4 / targetSibCode).getNorm();

    if (!isValid(projectileId, targetId, sqrtSnn)) {
      return {CrossSectionType::zero(), CrossSectionType::zero()};
    }

    double dummy, dum1, dum3, dum4, dumdif[3]; // dummies needed for fortran call
    int const iBeam = corsika::sibyll::getSibyllXSCode(
        projectileId); // 0 (can not interact, 1: proton-like, 2: pion-like,
                       // 3:kaon-like)

    double const dEcm = sqrtSnn / 1_GeV;
    // single nucleon target (p,n, hydrogen) or 4<=A<=18
    double sigProd = 0;
    double sigEla = 0;
    if (targetId == Code::Proton || targetId == Code::Hydrogen ||
        targetId == Code::Neutron) {
      // single nucleon target
      sib_sigma_hp_(iBeam, dEcm, dum1, sigEla, sigProd, dumdif, dum3, dum4);
    } else {
      // nuclear target
      int const iTarget = get_nucleus_A(targetId);
      sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy, sigEla);
    }
    return {sigProd * 1_mb, sigEla * 1_mb};
  } // namespace corsika::sibyll

  /**
   * In this function SIBYLL is called to produce one event. The
   * event is copied (and boosted) into the shower lab frame.
   */

  template <typename TSecondaryView>
  inline void HadronInteractionModel::doInteraction(TSecondaryView& secondaries,
                                              Code const projectileId,
                                              Code const targetId,
                                              FourMomentum const& projectileP4,
                                              FourMomentum const& targetP4) {

    int targetSibCode = 1; // nucleon or particle count
    if (is_nucleus(targetId)) { targetSibCode = get_nucleus_A(targetId); }
    CORSIKA_LOG_DEBUG("sibyll code: {} (nucleon/particle count)", targetSibCode);

    // sqrtS per target nucleon
    HEPEnergyType const sqrtSnn = (projectileP4 + targetP4 / targetSibCode).getNorm();
    COMBoost const boost(projectileP4, targetP4 / targetSibCode);

    if (!isValid(projectileId, targetId, sqrtSnn)) {
      throw std::runtime_error("Invalid target/projectile/energy combination");
    }

    CORSIKA_LOG_DEBUG("pId={} tId={} sqrtSnn={}GeV", projectileId, targetId, sqrtSnn);

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

    // link to sibyll stack
    SibStack ss;

    auto const& originalCS = boost.getOriginalCS();
    MomentumVector Plab_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
    for (auto& psib : ss) {
      // abort on particles that have decayed in Sibyll. Should not happen!
      if (psib.hasDecayed()) { // LCOV_EXCL_START
        throw std::runtime_error("found particle that decayed in SIBYLL!");
      } // LCOV_EXCL_STOP

      // transform 4-momentum to lab. frame
      // note that the momentum needs to be rotated back
      auto const tmp = psib.getMomentum().getComponents();
      auto const pCoM = MomentumVector(csPrime, tmp);
      HEPEnergyType const eCoM = psib.getEnergy();
      auto const P4lab = boost.fromCoM(FourVector{eCoM, pCoM});
      auto const p3lab = P4lab.getSpaceLikeComponents();

      Code const pid = corsika::sibyll::convertFromSibyll(psib.getPID());
      HEPEnergyType const mass = get_mass(pid);
      HEPEnergyType const Ekin = sqrt(p3lab.getSquaredNorm() + mass * mass) - mass;

      // add to corsika stack
      auto pnew =
          secondaries.addSecondary(std::make_tuple(pid, Ekin, p3lab.normalized()));

      Plab_final += pnew.getMomentum();
      Elab_final += pnew.getEnergy();
      Ecm_final += psib.getEnergy();
    }
    { // just output
      [[maybe_unused]] HEPEnergyType const Elab_initial =
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
  }
} // namespace corsika::sibyll
