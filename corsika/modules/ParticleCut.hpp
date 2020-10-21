/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/setup/SetupStack.hpp>

namespace corsika::particle_cut {

  class ParticleCut : public corsika::SecondariesProcess<ParticleCut> {

    HEPEnergyType const fECut;

    HEPEnergyType fEnergy = 0 * electronvolt;
    HEPEnergyType fEmEnergy = 0 * electronvolt;
    unsigned int fEmCount = 0;
    HEPEnergyType fInvEnergy = 0 * electronvolt;
    unsigned int fInvCount = 0;

  public:
    ParticleCut(const HEPEnergyType vCut)
        : fECut(vCut) {}

    bool ParticleIsInvisible(corsika::Code) const;
    EProcessReturn DoSecondaries(corsika::setup::StackView&);

    template <typename TParticle>
    bool ParticleIsBelowEnergyCut(TParticle const&) const;

    bool ParticleIsEmParticle(corsika::Code) const;

    void Init();
    void ShowResults();

    HEPEnergyType GetInvEnergy() const { return fInvEnergy; }
    HEPEnergyType GetCutEnergy() const { return fEnergy; }
    HEPEnergyType GetEmEnergy() const { return fEmEnergy; }
    unsigned int GetNumberEmParticles() const { return fEmCount; }
    unsigned int GetNumberInvParticles() const { return fInvCount; }
  };

} // namespace corsika::particle_cut

#include <corsika/detail/modules/ParticleCut.inl>
