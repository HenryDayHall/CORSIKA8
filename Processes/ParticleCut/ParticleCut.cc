/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/logging/Logging.h>
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

    template <typename TParticle>
    bool ParticleCut::checkCutParticle(const TParticle& particle) {

      const Code pid = particle.GetPID();
      HEPEnergyType energy = particle.GetEnergy();
      C8LOG_DEBUG(fmt::format("ParticleCut: checking {}, E= {} GeV, EcutTot={} GeV", pid,
                              energy / 1_GeV,
                              (fEmEnergy + fInvEnergy + fEnergy) / 1_GeV));
      if (bCutEm &&ParticleIsEmParticle(pid)) {
        C8LOG_DEBUG("removing em. particle...");
        fEmEnergy += energy;
        fEmCount += 1;
        return true;
      } else if (bCutInv && ParticleIsInvisible(pid)) {
        C8LOG_DEBUG("removing inv. particle...");
        fInvEnergy += energy;
        fInvCount += 1;
        return true;
      } else if (ParticleIsBelowEnergyCut(particle)) {
        C8LOG_DEBUG("removing low en. particle...");
        fEnergy += energy;
        return true;
      } else if (particle.GetTime() > 10_ms) {
        C8LOG_DEBUG("removing OLD particle...");
        fEnergy += energy;
        return true;
      }
      return false; // this particle will not be removed/cut
    }

    EProcessReturn ParticleCut::DoSecondaries(corsika::setup::StackView& vS) {
<<<<<<< HEAD
      auto particle = vS.begin();
      while (particle != vS.end()) {
        if (checkCutParticle(particle)) {
          particle.Delete();
=======
      auto p = vS.begin();
      while (p != vS.end()) {
        const Code pid = p.GetPID();
        HEPEnergyType energy = p.GetEnergy();
        if (discardEm_ && ParticleIsEmParticle(pid)) {
          emEnergy_ += energy;
          emCount_ += 1;
          p.Delete();
        } else if (discardInv_ && ParticleIsInvisible(pid)) {
          invEnergy_ += energy;
          invCount_ += 1;
          p.Delete();
        } else if (ParticleIsBelowEnergyCut(p)) {
          energy_ += energy;
          p.Delete();
        } else if (p.GetTime() > 10_ms) {
          energy_ += energy;
          p.Delete();
>>>>>>> dbc61589... rename ParticleCut cstr. bools because they were misunderstandable and delete delete debug messages
        } else {
          ++particle; // next entry in SecondaryView
        }
      }
      return EProcessReturn::eOk;
    }

<<<<<<< HEAD
    process::EProcessReturn ParticleCut::DoContinuous(Particle& particle, Track const&) {
      C8LOG_TRACE("ParticleCut::DoContinuous");
      if (checkCutParticle(particle)) {
        C8LOG_TRACE("removing during continuous");
        return process::EProcessReturn::eParticleAbsorbed;
      }
      return process::EProcessReturn::eOk;
    }

    ParticleCut::ParticleCut(const units::si::HEPEnergyType eCut, bool em, bool inv)
        : fECut(eCut)
        , bCutEm(em)
        , bCutInv(inv) {
=======
    ParticleCut::ParticleCut(const units::si::HEPEnergyType eCut, bool discardEm, bool discardInv)
        : eCut_(eCut)
        , discardEm_(cutEm)
        , discardInv_(discardInv) {
>>>>>>> dbc61589... rename ParticleCut cstr. bools because they were misunderstandable and delete delete debug messages

      fEmEnergy = 0_GeV;
      uiEmCount = 0;
      finvEnergy = 0_GeV;
      uiInvCount = 0;
      fEnergy = 0_GeV;
    }

    void ParticleCut::ShowResults() {
      C8LOG_INFO(fmt::format(
          " ******************************\n"
          " ParticleCut: \n"
          " energy in em.  component (GeV):  {}\n"
          " no. of em.  particles injected:  {}\n"
          " energy in inv. component (GeV):  {}\n"
          " no. of inv. particles injected:  {}\n"
          " energy below particle cut (GeV): {}\n"
          " ******************************",
          fEmEnergy / 1_GeV, uiEmCount, fInvEnergy / 1_GeV, uiInvCount, fEnergy / 1_GeV));
    }
  } // namespace particle_cut
} // namespace corsika::process
