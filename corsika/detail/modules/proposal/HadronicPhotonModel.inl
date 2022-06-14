/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/EnergyMomentumOperations.hpp>

#include <tuple>

namespace corsika::proposal {

  template <typename THadronicModel>
  inline HadronicPhotonModel<THadronicModel>::HadronicPhotonModel(
      THadronicModel& _hadint, HEPEnergyType const& _heenthresholdNN)
      : heHadronicInteraction_(_hadint)
      , heHadronicModelThresholdLabNN_(_heenthresholdNN) {
    // check validity of threshold assuming photon-nucleon
    // sqrtS per target nucleon
    HEPEnergyType const sqrtS =
        calculate_com_energy(_heenthresholdNN, Rho0::mass, Proton::mass);
    if (!heHadronicInteraction_.isValid(Code::Rho0, Code::Proton, sqrtS)) {
      CORSIKA_LOGGER_CRITICAL(
          logger_,
          "Invalid energy threshold for hadron interaction model. theshold_lab= {} GeV, "
          "theshold_com={} GeV",
          _heenthresholdNN / 1_GeV, sqrtS / 1_GeV);
      throw std::runtime_error("Configuration error!");
    }
    CORSIKA_LOGGER_DEBUG(
        logger_, "Threshold for HE hadronic interactions in proposal set to Elab={} GeV",
        _heenthresholdNN / 1_GeV);
  }

  template <typename THadronicModel>
  template <typename TStackView>
  inline ProcessReturn HadronicPhotonModel<THadronicModel>::doHadronicPhotonInteraction(
      TStackView& view, CoordinateSystemPtr const& labCS, FourMomentum const& photonP4,
      Code const& targetId) {
    if (photonP4.getTimeLikeComponent() > heHadronicModelThresholdLabNN_) {
      CORSIKA_LOGGER_TRACE(
          logger_, "HE photo-hadronic interaction! calling hadronic interaction model..");

      //  copy from sibyll::NuclearInteractionModel
      //  temporarily add to stack, will be removed after interaction in DoInteraction
      typename TStackView::inner_stack_value_type photonStack;
      Point const pDummy(labCS, {0_m, 0_m, 0_m});
      TimeType const tDummy = 0_ns;
      Code const hadPhotonCode = Code::Rho0; // stand in for hadronic-photon
      // target at rest
      FourMomentum const targetP4(get_mass(targetId),
                                  MomentumVector(labCS, {0_GeV, 0_GeV, 0_GeV}));
      auto hadronicPhoton = photonStack.addParticle(std::make_tuple(
          hadPhotonCode, photonP4.getTimeLikeComponent(),
          photonP4.getSpaceLikeComponents().normalized(), pDummy, tDummy));
      hadronicPhoton.setNode(view.getProjectile().getNode());
      // create inelastic interaction of the hadronic photon
      // create new StackView for the photon
      TStackView photon_secondaries(hadronicPhoton);

      // call inner hadronic event generator
      CORSIKA_LOGGER_TRACE(logger_, "{} + {} interaction. Ekinlab = {} GeV",
                           hadPhotonCode, targetId,
                           photonP4.getTimeLikeComponent() / 1_GeV);
      // check if had. model can handle configuration
      auto const sqrtSNN = (photonP4 + targetP4 / get_nucleus_A(targetId)).getNorm();
      // when Sibyll is used for hadronic interactions Argon cannot be used as target
      // nucleus. Since PROPOSAL has a non-zero cross section for Argon
      // targets we have to check here if the model can handle Argon (see Issue #498)
      if (!heHadronicInteraction_.isValid(hadPhotonCode, targetId, sqrtSNN)) {
        CORSIKA_LOGGER_WARN(
            logger_,
            "HE interaction model cannot handle configuration in photo-hadronic "
            "interaction! projectile={}, target={} (A={}, Z={}), sqrt(S) per "
            "nuc.={:8.2f} "
            "GeV. Skipping secondary production!",
            hadPhotonCode, targetId, get_nucleus_A(targetId), get_nucleus_Z(targetId),
            sqrtSNN / 1_GeV);
        return ProcessReturn::Ok;
      }
      heHadronicInteraction_.doInteraction(photon_secondaries, hadPhotonCode, targetId,
                                           photonP4, targetP4);
      for (const auto& pSec : photon_secondaries) {
        auto const p3lab = pSec.getMomentum();
        Code const pid = pSec.getPID();
        HEPEnergyType const secEkin =
            calculate_kinetic_energy(p3lab.getNorm(), get_mass(pid));
        view.addSecondary(std::make_tuple(pid, secEkin, p3lab.normalized()));
      }
    } else {
      CORSIKA_LOGGER_TRACE(
          logger_,
          "LE photo-hadronic interaction! Production of secondaries not implemented..");
    }
    return ProcessReturn::Ok;
  }
} // namespace corsika::proposal
