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
  inline HadronicPhotonModel<THadronicModel>::HadronicPhotonModel(THadronicModel& _hadint)
      : heHadronicInteraction_(_hadint){};

  template <typename THadronicModel>
  template <typename TStackView>
  inline ProcessReturn HadronicPhotonModel<THadronicModel>::doHadronicPhotonInteraction(
      TStackView& view, CoordinateSystemPtr const& labCS, FourMomentum const& photonP4,
      Code const& targetId) {
    if (photonP4.getTimeLikeComponent() > heHadronicModelThresholdLab_) {
      CORSIKA_LOG_INFO(
          "HE photo-hadronic interaction! calling hadronic interaction model..");

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
      CORSIKA_LOG_TRACE("calling HadronicInteraction...");
      CORSIKA_LOG_INFO("{} + {} interactions. Ekinlab = {} GeV", hadPhotonCode, targetId,
                       photonP4.getTimeLikeComponent() / 1_GeV);
      heHadronicInteraction_.doInteraction(photon_secondaries, hadPhotonCode, targetId,
                                           photonP4, targetP4);
      for (const auto& pSec : photon_secondaries) {
        auto const p3lab = pSec.getMomentum();
        Code const pid = pSec.getPID();
        HEPEnergyType const secEkin =
            calculate_kinetic_energy(p3lab.getNorm(), get_mass(pid));
        view.addSecondary(std::make_tuple(pid, secEkin, p3lab.normalized()));
      }
      CORSIKA_LOG_INFO("number of particles produced: {}", view.getEntries());
    } else {
      CORSIKA_LOG_INFO(
          "LE photo-hadronic interaction! production of secondaries not implemented..");
    }
    return ProcessReturn::Ok;
  }
} // namespace corsika::proposal