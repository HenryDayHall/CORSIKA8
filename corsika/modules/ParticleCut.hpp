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
#include <corsika/framework/process/ContinuousProcess.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika::particle_cut {

  class ParticleCut : public SecondariesProcess<ParticleCut>,
                      public ContinuousProcess<ParticleCut> {

  public:
    ParticleCut(const HEPEnergyType eCut, bool em, bool inv);

    void doSecondaries(corsika::setup::StackView&);
    ProcessReturn doContinuous(corsika::setup::Stack::particle_type& vParticle,
                               corsika::setup::Trajectory const& vTrajectory);
    LengthType getMaxStepLength(corsika::setup::Stack::particle_type const&,
                                corsika::setup::Trajectory const&) {
      return meter * std::numeric_limits<double>::infinity();
    }

    void showResults();
    void reset();

    HEPEnergyType getECut() const { return energy_cut_; }
    HEPEnergyType getInvEnergy() const { return inv_energy_; }
    HEPEnergyType getCutEnergy() const { return energy_; }
    HEPEnergyType getEmEnergy() const { return em_energy_; }
    unsigned int getNumberEmParticles() const { return em_count_; }
    unsigned int getNumberInvParticles() const { return inv_count_; }

  private:
    template <typename TParticle>
    bool checkCutParticle(const TParticle& p);

    template <typename TParticle>
    bool isBelowEnergyCut(TParticle const&) const;
    bool isEmParticle(Code) const;
    bool isInvisible(Code) const;

  private:
    HEPEnergyType energy_cut_;
    bool doCutEm_;
    bool doCutInv_;
    HEPEnergyType energy_ = 0 * electronvolt;
    HEPEnergyType em_energy_ = 0 * electronvolt;
    unsigned int em_count_ = 0;
    HEPEnergyType inv_energy_ = 0 * electronvolt;
    unsigned int inv_count_ = 0;
  };

} // namespace corsika::particle_cut

#include <corsika/detail/modules/ParticleCut.inl>
