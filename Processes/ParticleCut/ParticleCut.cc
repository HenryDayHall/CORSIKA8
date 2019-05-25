
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

    template <typename TParticle>
    bool ParticleCut::ParticleIsBelowEnergyCut(TParticle const& vP) const {
      auto const energyLab = vP.GetEnergy();
      // nuclei
      if (vP.GetPID() == particles::Code::Nucleus) {
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

    EProcessReturn ParticleCut::DoSecondaries(corsika::setup::StackView& vS) {
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
      fEmEnergy = 0._GeV;
      fEmCount = 0;
      fInvEnergy = 0._GeV;
      fInvCount = 0;
      fEnergy = 0._GeV;
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
