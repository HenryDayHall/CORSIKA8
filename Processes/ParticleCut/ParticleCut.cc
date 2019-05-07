
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/particle_cut/ParticleCut.h>

using namespace std;

using namespace corsika;
using namespace corsika::process;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace corsika::setup;

namespace corsika::process {
  namespace particle_cut {

    template <typename Particle>
    bool ParticleCut::ParticleIsBelowEnergyCut(Particle& vP) const {
      auto const energyLab = vP.GetEnergy();
      // nuclei
      if (vP.GetPID() == particles::Code::Nucleus) {
        auto const ElabNuc = energyLab / vP.GetNuclearA();
        auto const EcmNN = sqrt(2. * ElabNuc * 0.93827_GeV);
        if (ElabNuc < fECut || EcmNN < 10_GeV)
          return true;
        else
          return false;
      } else {
        // TODO: center-of-mass energy hard coded
        const HEPEnergyType Ecm = sqrt(2. * energyLab * 0.93827_GeV);
        if (energyLab < fECut || Ecm < 10_GeV)
          return true;
        else
          return false;
      }
    }

    bool ParticleCut::ParticleIsEmParticle(Code vCode) const {
      bool is_em = false;
      // FOR NOW: switch
      switch (vCode) {
        case Code::Electron:
          is_em = true;
          break;
        case Code::Positron:
          is_em = true;
          break;
        case Code::Gamma:
          is_em = true;
          break;
        default:
          break;
      }
      return is_em;
    }

    bool ParticleCut::ParticleIsInvisible(Code vCode) const {
      bool is_inv = false;
      // FOR NOW: switch
      switch (vCode) {
        case Code::NuE:
          is_inv = true;
          break;
        case Code::NuEBar:
          is_inv = true;
          break;
        case Code::NuMu:
          is_inv = true;
          break;
        case Code::NuMuBar:
          is_inv = true;
          break;
        case Code::MuPlus:
          is_inv = true;
          break;
        case Code::MuMinus:
          is_inv = true;
          break;

        case Code::Neutron:
          is_inv = true;
          break;

        case Code::AntiNeutron:
          is_inv = true;
          break;

        default:
          break;
      }
      return is_inv;
    }

    template <typename TSecondaries>
    EProcessReturn ParticleCut::DoSecondaries(TSecondaries& vS) {
      auto p = vS.begin();
      while (p != vS.end()) {
        const Code pid = p.GetPID();
        HEPEnergyType energy = p.GetEnergy();
        cout << "ProcessCut: DoSecondaries: " << pid << " E= " << energy
             << ", EcutTot=" << (fEmEnergy + fInvEnergy + fEnergy) / 1_GeV << " GeV"
             << endl;
        if (ParticleIsEmParticle(pid)) {
          cout << "removing em. particle..." << endl;
          fEmEnergy += energy;
          fEmCount += 1;
          p.Delete();
        } else if (ParticleIsInvisible(pid)) {
          cout << "removing inv. particle..." << endl;
          fInvEnergy += energy;
          fInvCount += 1;
          p.Delete();
        } else if (ParticleIsBelowEnergyCut(p)) {
          cout << "removing low en. particle..." << endl;
          fEnergy += energy;
          p.Delete();
        } else if (p.GetTime() > 10_ms) {
          cout << "removing OLD particle..." << endl;
          fEnergy += energy;
          p.Delete();
        } else {
          ++p; // next entry in SecondaryView
        }
      }
      return EProcessReturn::eOk;
    }

    void ParticleCut::Init() {
      fEmEnergy = 0. * 1_GeV;
      fEmCount = 0;
      fInvEnergy = 0. * 1_GeV;
      fInvCount = 0;
      fEnergy = 0. * 1_GeV;
      // defineEmParticles();
    }

    void ParticleCut::ShowResults() {
      cout << " ******************************" << endl
           << " ParticleCut: " << endl
           << " energy in em.  component (GeV):  " << fEmEnergy / 1_GeV << endl
           << " no. of em.  particles injected:  " << fEmCount << endl
           << " energy in inv. component (GeV):  " << fInvEnergy / 1_GeV << endl
           << " no. of inv. particles injected:  " << fInvCount << endl
           << " energy below particle cut (GeV): " << fEnergy / 1_GeV << endl
           << " ******************************" << endl;
    }
  } // namespace particle_cut
} // namespace corsika::process
