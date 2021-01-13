/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/ParticleCut.hpp>

namespace corsika {

  ParticleCut::ParticleCut(const HEPEnergyType eCut, bool em, bool inv)
      : energy_cut_(eCut)
      , doCutEm_(em)
      , doCutInv_(inv)
      , energy_(0_GeV)
      , em_energy_(0_GeV)
      , em_count_(0)
      , inv_energy_(0_GeV)
      , inv_count_(0) {}

  template <typename TParticle>
  bool ParticleCut::isBelowEnergyCut(TParticle const& vP) const {
    auto const energyLab = vP.getEnergy();
    // nuclei
    if (vP.getPID() == Code::Nucleus) {
      // calculate energy per nucleon
      auto const ElabNuc = energyLab / vP.getNuclearA();
      return (ElabNuc < energy_cut_);
    } else {
      return (energyLab < energy_cut_);
    }
  }

  template <typename TParticle>
  bool ParticleCut::checkCutParticle(const TParticle& particle) {

    const Code pid = particle.getPID();
    HEPEnergyType energy = particle.getEnergy();
    CORSIKA_LOG_DEBUG(fmt::format("ParticleCut: checking {}, E= {} GeV, EcutTot={} GeV",
                                  pid, energy / 1_GeV,
                                  (em_energy_ + inv_energy_ + energy_) / 1_GeV));
    if (doCutEm_ && is_em(pid)) {
      CORSIKA_LOG_DEBUG("removing em. particle...");
      em_energy_ += energy;
      em_count_ += 1;
      return true;
    } else if (doCutInv_ && is_neutrino(pid)) {
      CORSIKA_LOG_DEBUG("removing inv. particle...");
      inv_energy_ += energy;
      inv_count_ += 1;
      return true;
    } else if (isBelowEnergyCut(particle)) {
      CORSIKA_LOG_DEBUG("removing low en. particle...");
      energy_ += energy;
      return true;
    } else if (particle.getTime() > 10_ms) {
      CORSIKA_LOG_DEBUG("removing OLD particle...");
      energy_ += energy;
      return true;
    }
    return false; // this particle will not be removed/cut
  }

  void ParticleCut::doSecondaries(corsika::setup::StackView& vS) {
    auto particle = vS.begin();
    while (particle != vS.end()) {
      if (checkCutParticle(particle)) { particle.erase(); }
      ++particle; // next entry in SecondaryView
    }
  }

  ProcessReturn ParticleCut::doContinuous(corsika::setup::Stack::particle_type& particle,
                                          corsika::setup::Trajectory const&) {
    CORSIKA_LOG_TRACE("ParticleCut::DoContinuous");
    if (checkCutParticle(particle)) {
      CORSIKA_LOG_TRACE("removing during continuous");
      particle.erase();
      // signal to upstream code that this particle was deleted
      return ProcessReturn::ParticleAbsorbed;
    }
    return ProcessReturn::Ok;
  }

  void ParticleCut::showResults() {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " energy in em.  component (GeV): {} \n "
        " no. of em.  particles injected: {} \n "
        " energy in inv. component (GeV): {} \n "
        " no. of inv. particles injected: {} \n "
        " energy below particle cut (GeV): {} \n"
        " ******************************",
        em_energy_ / 1_GeV, em_count_, inv_energy_ / 1_GeV, inv_count_, energy_ / 1_GeV);
  }

  void ParticleCut::reset() {
    em_energy_ = 0_GeV;
    em_count_ = 0;
    inv_energy_ = 0_GeV;
    inv_count_ = 0;
    energy_ = 0_GeV;
  }

} // namespace corsika
