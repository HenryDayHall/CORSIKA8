/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {
  namespace particle_cut {
    class ParticleCut : public process::SecondariesProcess<ParticleCut> {

      units::si::HEPEnergyType const eCut_;
      bool discardEm_;
      bool discardInv_;

      units::si::HEPEnergyType energy_ = 0 * units::si::electronvolt;
      units::si::HEPEnergyType emEnergy_ = 0 * units::si::electronvolt;
      unsigned int emCount_ = 0;
      units::si::HEPEnergyType invEnergy_ = 0 * units::si::electronvolt;
      unsigned int invCount_ = 0;

    public:
      ParticleCut(const units::si::HEPEnergyType eCut, bool em, bool inv);

      bool ParticleIsInvisible(particles::Code) const;
      EProcessReturn DoSecondaries(corsika::setup::StackView&);

      template <typename TParticle>
      bool ParticleIsBelowEnergyCut(TParticle const&) const;

      bool ParticleIsEmParticle(particles::Code) const;

      void ShowResults();

      units::si::HEPEnergyType GetECut() const { return eCut_; }
      units::si::HEPEnergyType GetInvEnergy() const { return invEnergy_; }
      units::si::HEPEnergyType GetCutEnergy() const { return energy_; }
      units::si::HEPEnergyType GetEmEnergy() const { return emEnergy_; }
      unsigned int GetNumberEmParticles() const { return emCount_; }
      unsigned int GetNumberInvParticles() const { return invCount_; }
    };
  } // namespace particle_cut
} // namespace corsika::process
