/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/SecondariesProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process {
  namespace particle_cut {
    class ParticleCut : public process::SecondariesProcess<ParticleCut>,
                        public corsika::process::ContinuousProcess<ParticleCut> {

      units::si::HEPEnergyType const fECut_;
      bool bCutEm;
      bool bCutInv;

      units::si::HEPEnergyType fEnergy = 0 * units::si::electronvolt;
      units::si::HEPEnergyType fEmEnergy = 0 * units::si::electronvolt;
      unsigned int uiEmCount = 0;
      units::si::HEPEnergyType fInvEnergy = 0 * units::si::electronvolt;
      unsigned int uiInvCount = 0;

    public:
      ParticleCut(const units::si::HEPEnergyType eCut, bool em, bool inv);

      EProcessReturn DoSecondaries(corsika::setup::StackView&);

      EProcessReturn DoContinuous(Particle& vParticle, Track const& vTrajectory);

      corsika::units::si::LengthType MaxStepLength(
          corsika::setup::Stack::ParticleType const&, corsika::setup::Trajectory const&) {
        return units::si::meter * std::numeric_limits<double>::infinity();
      }

      void ShowResults();

    protected:
      template <typename TParticle>
      bool checkCutParticle(const TParticle& p);

      template <typename TParticle>
      bool ParticleIsBelowEnergyCut(TParticle const&) const;

      bool ParticleIsEmParticle(particles::Code) const;

      void ShowResults();

      units::si::HEPEnergyType GetECut() const { return fECut; }
      units::si::HEPEnergyType GetInvEnergy() const { return fInvEnergy; }
      units::si::HEPEnergyType GetCutEnergy() const { return fEnergy; }
      units::si::HEPEnergyType GetEmEnergy() const { return fEmEnergy; }
      unsigned int GetNumberEmParticles() const { return uiEmCount; }
      unsigned int GetNumberInvParticles() const { return fInvCount; }
    };
  } // namespace particle_cut
} // namespace corsika::process
