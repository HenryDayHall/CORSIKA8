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

  template <typename TOutput>
  template <typename... TArgs>
  inline ParticleCut<TOutput>::ParticleCut(HEPEnergyType const eEleCut,
                                           HEPEnergyType const ePhoCut,
                                           HEPEnergyType const eHadCut,
                                           HEPEnergyType const eMuCut, bool const inv,
                                           bool const em, TArgs&&... args)
      : TOutput(std::forward<TArgs>(args)...)
      , doCutInv_(inv)
      , doCutEm_(em)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_invcut_(0_GeV)
      , inv_count_(0)
      , em_count_(0)
      , energy_count_() {
    for (auto p : get_all_particles()) {
      if (is_hadron(p)) // nuclei are also hadrons
        set_kinetic_energy_threshold(p, eHadCut);
      else if (is_muon(p))
        set_kinetic_energy_threshold(p, eMuCut);
      else if (p == Code::Electron || p == Code::Positron)
        set_kinetic_energy_threshold(p, eEleCut);
      else if (p == Code::Photon)
        set_kinetic_energy_threshold(p, ePhoCut);
    }
    set_kinetic_energy_threshold(Code::Nucleus, eHadCut);
    CORSIKA_LOG_DEBUG(
        "setting kinetic energy thresholds: electrons = {} GeV, photons = {} GeV, "
        "hadrons = {} GeV, "
        "muons = {} GeV",
        eEleCut / 1_GeV, ePhoCut / 1_GeV, eHadCut / 1_GeV, eMuCut / 1_GeV);
  }

  template <typename TOutput>
  template <typename... TArgs>
  inline ParticleCut<TOutput>::ParticleCut(HEPEnergyType const eCut, bool const inv,
                                           bool const em, TArgs&&... args)
      : TOutput(std::forward<TArgs>(args)...)
      , doCutInv_(inv)
      , doCutEm_(em)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_invcut_(0_GeV)
      , energy_emcut_(0_GeV)
      , inv_count_(0)
      , em_count_(0)
      , energy_count_() {
    for (auto p : get_all_particles()) { set_kinetic_energy_threshold(p, eCut); }
    set_kinetic_energy_threshold(Code::Nucleus, eCut);
    CORSIKA_LOG_DEBUG("setting kinetic energy threshold {} GeV", eCut / 1_GeV);
  }

  template <typename TOutput>
  template <typename... TArgs>
  inline ParticleCut<TOutput>::ParticleCut(
      std::unordered_map<Code const, HEPEnergyType const> const& eCuts, bool const inv,
      bool const em, TArgs&&... args)
      : TOutput(std::forward<TArgs>(args)...)
      , doCutInv_(inv)
      , doCutEm_(em)
      , energy_cut_(0_GeV)
      , energy_timecut_(0_GeV)
      , energy_invcut_(0_GeV)
      , energy_emcut_(0_GeV)
      , inv_count_(0)
      , em_count_(0)
      , energy_count_(0) {
    set_kinetic_energy_thresholds(eCuts);
    CORSIKA_LOG_DEBUG("setting threshold particles individually");
  }

  template <typename TOutput>
  template <typename TParticle>
  inline bool ParticleCut<TOutput>::isBelowEnergyCut(TParticle const& vP) const {
    auto const energyLab = vP.getKineticEnergy();
    auto const pid = vP.getPID();
    // nuclei
    if (is_nucleus(pid)) {
      // calculate energy per nucleon
      auto const ElabNuc = energyLab / get_nucleus_A(pid);
      return (ElabNuc < get_kinetic_energy_threshold(pid));
    } else {
      return (energyLab < get_kinetic_energy_threshold(pid));
    }
  }

  template <typename TOutput>
  inline bool ParticleCut<TOutput>::isInvisible(Code const& vCode) const {
    return is_neutrino(vCode);
  }

  template <typename TOutput>
  template <typename TParticle>
  inline bool ParticleCut<TOutput>::checkCutParticle(TParticle const& particle) {

    Code const pid = particle.getPID();
    HEPEnergyType const kine_energy = particle.getKineticEnergy();
    HEPEnergyType const energy = particle.getEnergy();
    CORSIKA_LOG_DEBUG(
        "ParticleCut: checking {} ({}), E_kin= {} GeV, EcutTot={} GeV, E={} GeV, m={} "
        "GeV",
        pid, particle.getPDG(), kine_energy / 1_GeV,
        (energy_emcut_ + energy_invcut_ + energy_cut_ + energy_timecut_) / 1_GeV,
        energy / 1_GeV, particle.getMass() / 1_GeV);
    CORSIKA_LOG_DEBUG("p={}", particle.asString());
    if (doCutEm_ && is_em(pid)) {
      CORSIKA_LOG_DEBUG("removing em. particle...");
      energy_emcut_ += kine_energy;
      em_count_ += 1;
      energy_event_ += kine_energy;
      return true;
    } else if (doCutInv_ && is_neutrino(pid)) {
      CORSIKA_LOG_DEBUG("removing inv. particle...");
      energy_invcut_ += kine_energy;
      inv_count_ += 1;
      energy_event_ += kine_energy;
      return true;
    } else if (isBelowEnergyCut(particle)) {
      CORSIKA_LOG_DEBUG("removing low en. particle...");
      energy_cut_ += kine_energy;
      energy_count_ += 1;
      energy_event_ += kine_energy;
      return true;
    } else if (particle.getTime() > 10_ms) {
      CORSIKA_LOG_DEBUG("removing OLD particle...");
      energy_timecut_ += kine_energy;
      energy_event_ += kine_energy;
      return true;
    }
    return false; // this particle will not be removed/cut
  }

  template <typename TOutput>
  template <typename TStackView>
  inline void ParticleCut<TOutput>::doSecondaries(TStackView& vS) {
    energy_event_ = 0_GeV; // per event counting for printout
    auto particle = vS.begin();
    while (particle != vS.end()) {
      if (checkCutParticle(particle)) {
        this->write(particle.getPosition(), particle.getPID(),
                    particle.getKineticEnergy());
        particle.erase();
      }
      ++particle; // next entry in SecondaryView
    }
    CORSIKA_LOG_DEBUG("Event cut: {} GeV", energy_event_ / 1_GeV);
  }

  template <typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn ParticleCut<TOutput>::doContinuous(TParticle& particle,
                                                          TTrajectory const&,
                                                          bool const) {
    if (checkCutParticle(particle)) {
      this->write(particle.getPosition(), particle.getPID(), particle.getKineticEnergy());
      CORSIKA_LOG_TRACE("removing during continuous");
      // signal to upstream code that this particle was deleted
      return ProcessReturn::ParticleAbsorbed;
    }
    return ProcessReturn::Ok;
  }

  template <typename TOutput>
  inline void ParticleCut<TOutput>::printThresholds() const {
    for (auto p : get_all_particles()) {
      auto const Eth = get_kinetic_energy_threshold(p);
      CORSIKA_LOG_DEBUG("kinetic energy threshold for particle {} is {} GeV", p,
                        Eth / 1_GeV);
    }
  }

  template <typename TOutput>
  inline void ParticleCut<TOutput>::showResults() const {
    CORSIKA_LOG_INFO(
        "\n ******************************\n "
        " kinetic energy removed by cut of electromagnetic (GeV): {} (number: {})\n "
        " kinetic energy removed by cut of invisible (GeV): {} (number: {})\n "
        " kinetic energy removed by kinetic energy cut (GeV): {} (number: {}) \n "
        " kinetic energy removed by time cut (GeV): {} \n"
        " ******************************",
        energy_emcut_ / 1_GeV, em_count_, energy_invcut_ / 1_GeV, inv_count_,
        energy_cut_ / 1_GeV, energy_count_, energy_timecut_ / 1_GeV);
  }

  template <typename TOutput>
  inline void ParticleCut<TOutput>::reset() {
    energy_invcut_ = 0_GeV;
    inv_count_ = 0;
    energy_cut_ = 0_GeV;
    energy_count_ = 0;
    energy_timecut_ = 0_GeV;
  }

  template <typename TOutput>
  inline YAML::Node ParticleCut<TOutput>::getConfig() const {

    YAML::Node node;
    node["type"] = "ParticleCut";
    node["units"]["energy"] = "GeV";
    node["energy_invcut"] = energy_invcut_ / 1_GeV;
    node["inv_count"] = inv_count_;
    node["energy_cut"] = energy_cut_ / 1_GeV;
    node["energy_timecut_"] = energy_timecut_ / 1_GeV;

    return node;
  }

} // namespace corsika
