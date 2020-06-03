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
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {
  namespace particle_cut {
    class ParticleCut : public process::SecondariesProcess<ParticleCut> {

      units::si::HEPEnergyType const fECut;

      units::si::HEPEnergyType fEnergy = 0 * units::si::electronvolt;
      units::si::HEPEnergyType fEmEnergy = 0 * units::si::electronvolt;
      unsigned int fEmCount = 0;
      units::si::HEPEnergyType fInvEnergy = 0 * units::si::electronvolt;
      unsigned int fInvCount = 0;

    public:
      ParticleCut(const units::si::HEPEnergyType vCut)
          : fECut(vCut) {}

      bool ParticleIsInvisible(particles::Code) const;
      EProcessReturn DoSecondaries(corsika::setup::StackView&);

      template <typename TParticle>
      bool ParticleIsBelowEnergyCut(TParticle const&) const;

      bool ParticleIsEmParticle(particles::Code) const;

      void Init();
      void ShowResults();

      units::si::HEPEnergyType GetInvEnergy() const { return fInvEnergy; }
      units::si::HEPEnergyType GetCutEnergy() const { return fEnergy; }
      units::si::HEPEnergyType GetEmEnergy() const { return fEmEnergy; }
      unsigned int GetNumberEmParticles() const { return fEmCount; }
      unsigned int GetNumberInvParticles() const { return fInvCount; }
    };
  } // namespace particle_cut
} // namespace corsika::process

#endif
