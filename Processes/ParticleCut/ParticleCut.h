/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_particle_cut_ParticleCut_h_
#define _corsika_process_particle_cut_ParticleCut_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika;
using namespace corsika::units;
using namespace corsika::units::si;

namespace corsika::process {
  namespace particle_cut {
    class ParticleCut : public process::SecondariesProcess<ParticleCut> {

      HEPEnergyType fECut;

      HEPEnergyType fEnergy = 0_GeV;
      HEPEnergyType fEmEnergy = 0_GeV;
      int fEmCount = 0;
      HEPEnergyType fInvEnergy = 0_GeV;
      int fInvCount = 0;

    public:
      ParticleCut(const HEPEnergyType vCut)
          : fECut(vCut) {}

      bool ParticleIsInvisible(particles::Code) const;
      template <typename TSecondaries>
      EProcessReturn DoSecondaries(TSecondaries&);

      template <typename Particle>
      bool ParticleIsBelowEnergyCut(Particle&) const;

      bool ParticleIsEmParticle(particles::Code) const;

      void Init();
      void ShowResults();

      HEPEnergyType GetInvEnergy() const { return fInvEnergy; }
      HEPEnergyType GetCutEnergy() const { return fEnergy; }
      HEPEnergyType GetEmEnergy() const { return fEmEnergy; }
    };
  } // namespace particlecut
} // namespace corsika::process

#endif
