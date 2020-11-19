/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/ParticleCut.hpp>

namespace corsika::particle_cut {

  template <typename TParticle>
  bool ParticleCut::ParticleIsBelowEnergyCut(TParticle const& vP) const {
    auto const energyLab = vP.GetEnergy();
    // nuclei
    if (vP.GetPID() == corsika::Code::Nucleus) {
      // calculate energy per nucleon
      auto const ElabNuc = energyLab / vP.GetNuclearA();
      return (ElabNuc < fECut);
    } else {
      return (energyLab < fECut);
    }
  }

  bool ParticleCut::ParticleIsEmParticle(Code vCode) const {
    // FOR NOW: switch
    switch (vCode) {
      case Code::Gamma:
      case Code::Electron:
      case Code::Positron:
        return true;
      default:
        return false;
    }
  }

  bool ParticleCut::ParticleIsInvisible(Code vCode) const {
    switch (vCode) {
      case Code::NuE:
      case Code::NuEBar:
      case Code::NuMu:
      case Code::NuMuBar:
        return true;

      default:
        return false;
    }
  }

  void ParticleCut::doSecondaries(corsika::setup::StackView& vS) {

    auto p = vS.begin();
    while (p != vS.end()) {
      const Code pid = p.GetPID();
      HEPEnergyType energy = p.GetEnergy();
      std::cout << "ProcessCut: DoSecondaries: " << pid << " E= " << energy
                << ", EcutTot=" << (fEmEnergy + fInvEnergy + fEnergy) / 1_GeV << " GeV"
                << std::endl;
      if (ParticleIsEmParticle(pid)) {
        std::cout << "removing em. particle..." << std::endl;
        fEmEnergy += energy;
        fEmCount += 1;
        p.Delete();
      } else if (ParticleIsInvisible(pid)) {
        std::cout << "removing inv. particle..." << std::endl;
        fInvEnergy += energy;
        fInvCount += 1;
        p.Delete();
      } else if (ParticleIsBelowEnergyCut(p)) {
        std::cout << "removing low en. particle..." << std::endl;
        fEnergy += energy;
        p.Delete();
      } else if (p.GetTime() > 10_ms) {
        std::cout << "removing OLD particle..." << std::endl;
        fEnergy += energy;
        p.Delete();
      } else {
        ++p; // next entry in SecondaryView
      }
    }
  }

  void ParticleCut::Init() {
    fEmEnergy = 0_GeV;
    fEmCount = 0;
    fInvEnergy = 0_GeV;
    fInvCount = 0;
    fEnergy = 0_GeV;
    // defineEmParticles();
  }

  void ParticleCut::ShowResults() {
    std::cout << " ******************************" << std::endl
              << " ParticleCut: " << std::endl
              << " energy in em.  component (GeV):  " << fEmEnergy / 1_GeV << std::endl
              << " no. of em.  particles injected:  " << fEmCount << std::endl
              << " energy in inv. component (GeV):  " << fInvEnergy / 1_GeV << std::endl
              << " no. of inv. particles injected:  " << fInvCount << std::endl
              << " energy below particle cut (GeV): " << fEnergy / 1_GeV << std::endl
              << " ******************************" << std::endl;
  }

} // namespace corsika::particle_cut
