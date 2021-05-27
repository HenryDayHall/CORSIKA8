/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>

namespace corsika {

  inline ParticleCut::ParticleCut(HEPEnergyType const eEleCut,
                                  HEPEnergyType const ePhoCut,
                                  HEPEnergyType const eHadCut, HEPEnergyType const eMuCut,
                                  bool const inv)
      : doCutEm_(false)
      , doCutInv_(inv)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_emcut_(0_GeV)
      , energy_invcut_(0_GeV)
      , em_count_(0)
      , inv_count_(0) {
    for (auto p : get_all_particles())
      if (is_hadron(p)) // nuclei are also hadrons
        set_kinetic_energy_threshold(p, eHadCut);
      else if (is_muon(p))
        set_kinetic_energy_threshold(p, eMuCut);
      else if (p == Code::Electron || p == Code::Positron)
        set_kinetic_energy_threshold(p, eEleCut);
      else if (p == Code::Photon)
        set_kinetic_energy_threshold(p, ePhoCut);
    CORSIKA_LOG_DEBUG(
        "setting kinetic energy thresholds: electrons = {} GeV, photons = {} GeV, "
        "hadrons = {} GeV, "
        "muons = {} GeV",
        eEleCut / 1_GeV, ePhoCut / 1_GeV, eHadCut / 1_GeV, eMuCut / 1_GeV);
    printThresholds();
  }

  inline ParticleCut::ParticleCut(HEPEnergyType const eHadCut, HEPEnergyType const eMuCut,
                                  bool const inv)
      : doCutEm_(true)
      , doCutInv_(inv)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_emcut_(0_GeV)
      , energy_invcut_(0_GeV)
      , em_count_(0)
      , inv_count_(0) {

    for (auto p : get_all_particles())
      if (is_hadron(p))
        set_kinetic_energy_threshold(p, eHadCut);
      else if (is_muon(p))
        set_kinetic_energy_threshold(p, eMuCut);
    CORSIKA_LOG_DEBUG(
        "setting thresholds: hadrons = {} GeV, "
        "muons = {} GeV",
        eHadCut / 1_GeV, eMuCut / 1_GeV);
    printThresholds();
  }

  inline ParticleCut::ParticleCut(HEPEnergyType const eCut, bool const em, bool const inv)
      : doCutEm_(em)
      , doCutInv_(inv)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_emcut_(0_GeV)
      , energy_invcut_(0_GeV)
      , em_count_(0)
      , inv_count_(0) {
    for (auto p : get_all_particles()) set_kinetic_energy_threshold(p, eCut);
    CORSIKA_LOG_DEBUG("setting kinetic energy threshold for all particles to {} GeV",
                      eCut / 1_GeV);
    printThresholds();
  }

  inline ParticleCut::ParticleCut(
      std::unordered_map<Code const, HEPEnergyType const> const& eCuts, bool const em,
      bool const inv)
      : doCutEm_(em)
      , doCutInv_(inv)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_emcut_(0_GeV)
      , energy_invcut_(0_GeV)
      , em_count_(0)
      , inv_count_(0) {
    set_kinetic_energy_thresholds(eCuts);
    CORSIKA_LOG_DEBUG("setting threshold particles individually");
    printThresholds();
  }

  template <typename TParticle>
  inline bool ParticleCut::isBelowEnergyCut(TParticle const& vP) const {
    auto const energyLab = vP.getKineticEnergy();
    auto const pid = vP.getPID();
    // nuclei
    if (pid == Code::Nucleus) {
      // calculate energy per nucleon
      auto const ElabNuc = energyLab / vP.getNuclearA();
      return (ElabNuc < get_kinetic_energy_threshold(pid));
    } else {
      return (energyLab < get_kinetic_energy_threshold(pid));
    }
  }

  inline bool ParticleCut::isInvisible(Code const& vCode) const {
    return is_neutrino(vCode);
  }

  template <typename TParticle>
  inline bool ParticleCut::checkCutParticle(TParticle const& particle) {

    Code const pid = particle.getPID();
    HEPEnergyType const kine_energy = particle.getKineticEnergy();
    HEPEnergyType const energy = particle.getEnergy();
    CORSIKA_LOG_DEBUG(
        "ParticleCut: checking {}, E_kin= {} GeV, EcutTot={} GeV", pid,
        kine_energy / 1_GeV,
        (energy_emcut_ + energy_invcut_ + energy_cut_ + energy_timecut_) / 1_GeV);
    if (doCutEm_ && is_em(pid)) {
      CORSIKA_LOG_DEBUG("removing em. particle...");
      energy_emcut_ += energy;
      em_count_ += 1;
      return true;
    } else if (doCutInv_ && is_neutrino(pid)) {
      CORSIKA_LOG_DEBUG("removing inv. particle...");
      energy_invcut_ += energy;
      inv_count_ += 1;
      return true;
    } else if (isBelowEnergyCut(particle)) {
      CORSIKA_LOG_DEBUG("removing low en. particle...");
      energy_cut_ += energy;
      return true;
    } else if (particle.getTime() > 10_ms) {
      CORSIKA_LOG_DEBUG("removing OLD particle...");
      energy_timecut_ += energy;
      return true;
    }
    return false; // this particle will not be removed/cut
  }

  template <typename TStackView>
  inline void ParticleCut::doSecondaries(TStackView& vS) {
    auto particle = vS.begin();
    while (particle != vS.end()) {
      if (checkCutParticle(particle)) { particle.erase(); }
      ++particle; // next entry in SecondaryView
    }
  }

  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn ParticleCut::doContinuous(TParticle& particle, TTrajectory const&,
                                                 bool const) {
    CORSIKA_LOG_TRACE("ParticleCut::DoContinuous");
    if (checkCutParticle(particle)) {
      CORSIKA_LOG_TRACE("removing during continuous");
      // signal to upstream code that this particle was deleted
      return ProcessReturn::ParticleAbsorbed;
    }
    return ProcessReturn::Ok;
  }

  inline void ParticleCut::printThresholds() {
    for (auto p : get_all_particles()) {
      auto const Eth = get_kinetic_energy_threshold(p);
      CORSIKA_LOG_INFO("kinetic energy threshold for particle {} is {} GeV", p,
                       Eth / 1_GeV);
    }
  }

  inline void ParticleCut::showResults() {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " energy removed by cut of electromagnetic (GeV): {} \n "
        " no. of em.  particles removed : {} \n "
        " energy removed by cut of invisible (GeV): {} \n "
        " no. of invisible particles removed : {} \n "
        " energy removed by kinetic energy cut (GeV): {} \n"
        " energy removed by time cut (GeV): {} \n"
        " ******************************",
        energy_emcut_ / 1_GeV, em_count_, energy_invcut_ / 1_GeV, inv_count_,
        energy_cut_ / 1_GeV, energy_timecut_ / 1_GeV);
  }

  inline void ParticleCut::reset() {
    energy_emcut_ = 0_GeV;
    em_count_ = 0;
    energy_invcut_ = 0_GeV;
    inv_count_ = 0;
    energy_cut_ = 0_GeV;
    energy_timecut_ = 0_GeV;
  }

} // namespace corsika
