/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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
        return (ElabNuc < eCut_);
      } else {
        return (energyLab < eCut_);
      }
    }

    bool ParticleCut::ParticleIsEmParticle(Code vCode) const {
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

    EProcessReturn ParticleCut::DoSecondaries(corsika::setup::StackView& vS) {
      auto p = vS.begin();
      while (p != vS.end()) {
        const Code pid = p.GetPID();
        HEPEnergyType energy = p.GetEnergy();
        cout << "ProcessCut: DoSecondaries: " << pid << " E= " << energy
             << ", EcutTot=" << (emEnergy_ + invEnergy_ + energy_) / 1_GeV << " GeV"
             << endl;
        if (cutEm_ && ParticleIsEmParticle(pid)) {
          cout << "removing em. particle..." << endl;
          emEnergy_ += energy;
          emCount_ += 1;
          p.Delete();
        } else if (cutInv_ && ParticleIsInvisible(pid)) {
          cout << "removing inv. particle..." << endl;
          invEnergy_ += energy;
          invCount_ += 1;
          p.Delete();
        } else if (ParticleIsBelowEnergyCut(p)) {
          cout << "removing low en. particle..." << endl;
          energy_ += energy;
          p.Delete();
        } else if (p.GetTime() > 10_ms) {
          cout << "removing OLD particle..." << endl;
          energy_ += energy;
          p.Delete();
        } else {
          ++p; // next entry in SecondaryView
        }
      }
      return EProcessReturn::eOk;
    }

    ParticleCut::ParticleCut(const units::si::HEPEnergyType eCut, bool em, bool inv)
        : eCut_(eCut)
        , cutEm_(em)
        , cutInv_(inv) {

      emEnergy_ = 0_GeV;
      emCount_ = 0;
      invEnergy_ = 0_GeV;
      invCount_ = 0;
      energy_ = 0_GeV;
    }

    void ParticleCut::ShowResults() {
      cout << " ******************************" << endl
           << " ParticleCut: " << endl
           << " energy in em.  component (GeV):  " << emEnergy_ / 1_GeV << endl
           << " no. of em.  particles injected:  " << emCount_ << endl
           << " energy in inv. component (GeV):  " << invEnergy_ / 1_GeV << endl
           << " no. of inv. particles injected:  " << invCount_ << endl
           << " energy below particle cut (GeV): " << energy_ / 1_GeV << endl
           << " ******************************" << endl;
    }
  } // namespace particle_cut
} // namespace corsika::process
