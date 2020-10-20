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

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/sequence/SecondariesProcess.hpp>
#include <corsika/setup/SetupStack.hpp>

namespace corsika::particle_cut {

  class ParticleCut : public corsika::SecondariesProcess<ParticleCut> {

    units::si::HEPEnergyType const fECut;

    units::si::HEPEnergyType fEnergy = 0 * units::si::electronvolt;
    units::si::HEPEnergyType fEmEnergy = 0 * units::si::electronvolt;
    unsigned int fEmCount = 0;
    units::si::HEPEnergyType fInvEnergy = 0 * units::si::electronvolt;
    unsigned int fInvCount = 0;

  public:
    ParticleCut(const units::si::HEPEnergyType vCut)
        : fECut(vCut) {}

    bool ParticleIsInvisible(corsika::Code) const;
    EProcessReturn DoSecondaries(corsika::setup::StackView&);

    template <typename TParticle>
    bool ParticleIsBelowEnergyCut(TParticle const&) const;

    bool ParticleIsEmParticle(corsika::Code) const;

    void Init();
    void ShowResults();

    units::si::HEPEnergyType GetInvEnergy() const { return fInvEnergy; }
    units::si::HEPEnergyType GetCutEnergy() const { return fEnergy; }
    units::si::HEPEnergyType GetEmEnergy() const { return fEmEnergy; }
    unsigned int GetNumberEmParticles() const { return fEmCount; }
    unsigned int GetNumberInvParticles() const { return fInvCount; }
  };

} // namespace corsika::particle_cut

#include <corsika/detail/modules/particle_cut/ParticleCut.inl>
