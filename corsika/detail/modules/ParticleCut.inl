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
                                           TArgs&&... outputArgs)
      : TOutput(std::forward<TArgs>(outputArgs)...)
      , cut_electrons_(eEleCut)
      , cut_photons_(ePhoCut)
      , cut_muons_(eMuCut)
      , cut_hadrons_(eHadCut)
      , doCutInv_(inv) {
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
                                           TArgs&&... outputArgs)
      : TOutput(std::forward<TArgs>(outputArgs)...)
      , doCutInv_(inv) {
    for (auto p : get_all_particles()) { set_kinetic_energy_threshold(p, eCut); }
    set_kinetic_energy_threshold(Code::Nucleus, eCut);
    CORSIKA_LOG_DEBUG("setting kinetic energy threshold {} GeV", eCut / 1_GeV);
  }

  template <typename TOutput>
  template <typename... TArgs>
  inline ParticleCut<TOutput>::ParticleCut(
      std::unordered_map<Code const, HEPEnergyType const> const& eCuts, bool const inv,
      TArgs&&... args)
      : TOutput(std::forward<TArgs>(args)...)
      , doCutInv_(inv) {
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
  template <typename TParticle>
  inline bool ParticleCut<TOutput>::checkCutParticle(TParticle const& particle) {

    Code const pid = particle.getPID();
    HEPEnergyType const kine_energy = particle.getKineticEnergy();
    HEPEnergyType const energy = particle.getEnergy();
    CORSIKA_LOG_DEBUG(
        "ParticleCut: checking {} ({}), E_kin= {} GeV, E={} GeV, m={} "
        "GeV",
        pid, particle.getPDG(), kine_energy / 1_GeV, energy / 1_GeV,
        particle.getMass() / 1_GeV);
    CORSIKA_LOG_DEBUG("p={}", particle.asString());
    if (doCutInv_ && is_neutrino(pid)) {
      CORSIKA_LOG_DEBUG("removing inv. particle...");
      return true;
    } else if (isBelowEnergyCut(particle)) {
      CORSIKA_LOG_DEBUG("removing low en. particle...");
      return true;
    } else if (particle.getTime() > 10_ms) {
      CORSIKA_LOG_DEBUG("removing OLD particle...");
      return true;
    } else {
      for (auto const& cut : cuts_) {
        if (pid == cut.first && kine_energy < cut.second) { return true; }
      }
    }
    return false; // this particle will not be removed/cut
  }

  template <typename TOutput>
  template <typename TStackView>
  inline void ParticleCut<TOutput>::doSecondaries(TStackView& vS) {
    HEPEnergyType energy_event = 0_GeV; // per event counting for printout
    auto particle = vS.begin();
    while (particle != vS.end()) {
      if (checkCutParticle(particle)) {
        HEPEnergyType Ekin = particle.getKineticEnergy();
        this->write(particle.getPosition(), particle.getPID(), Ekin);
        particle.erase();
      }
      ++particle; // next entry in SecondaryView
    }
    CORSIKA_LOG_DEBUG("Event cut: {} GeV", energy_event / 1_GeV);
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

    CORSIKA_LOG_DEBUG("kinetic energy threshold for electrons is {} GeV",
                      cut_electrons_ / 1_GeV);
    CORSIKA_LOG_DEBUG("kinetic energy threshold for photons is {} GeV",
                      cut_photons_ / 1_GeV);
    CORSIKA_LOG_DEBUG("kinetic energy threshold for muons is {} GeV", cut_muons_ / 1_GeV);
    CORSIKA_LOG_DEBUG("kinetic energy threshold for hadros is {} GeV",
                      cut_hadrons_ / 1_GeV);

    for (auto const& cut : cuts_) {
      CORSIKA_LOG_DEBUG("kinetic energy threshold for particle {} is {} GeV", cut.first,
                        cut.second / 1_GeV);
    }
  }

  template <typename TOutput>
  inline YAML::Node ParticleCut<TOutput>::getConfig() const {

    YAML::Node node;
    node["type"] = "ParticleCut";
    node["units"]["energy"] = "GeV";
    node["cut_electrons"] = cut_electrons_ / 1_GeV;
    node["cut_photons"] = cut_photons_ / 1_GeV;
    node["cut_muons"] = cut_muons_ / 1_GeV;
    node["cut_hadrons"] = cut_hadrons_ / 1_GeV;
    node["cut_invisibles"] = doCutInv_;
    for (auto const& cut : cuts_) {
      node[fmt::format("cut_{}", cut.first)] = cut.second / 1_GeV;
    }
    return node;
  }

} // namespace corsika
